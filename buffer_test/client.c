#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define DEBUG_BUILD
#ifdef DEBUG_BUILD
#define debug(fmt, ...) printf( fmt,  ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

typedef struct message_buffer {
    char *buffer;
    int w_p;
    int r_p;
} MessageBuffer;


#define SERVER_NAME "localhost"
#define PORT_NUM 10000

//接收线程配置
#define RECEIVE_DATA_HEADER_LENGTH 4

#define RECEIVE_DATA_BUFFER_SIZE (8 * 1024)
#define RECEIVE_NORMAL_INTERVAL (100 * 1000)  //没有数据到达时的轮询时间　以微秒计算）
#define BUFFER_UPDATE_INTERVAL 1    //缓冲区更新轮询时间（以秒来计算）

//消息处理线程配置
#define MESSAGE_PROCESS_BUFFER_SIZE (1024 * 1024)
#define MESSAGE_PROCESS_DATA_SIZE (64 * 1024)
#define MESSAGE_PROCESS_INTERVAL (100 * 1000)  //消息处理轮询时间（以微秒计算）
#define MESSAGE_WAIT_INTERVAL (10 * 1000)    //消息不完整，等待消息完整的轮询时间（以微秒计算）

#define SEND_BUILD_CONNECTION_HEADER 1
#define SEND_HEART_BREAK_HEADER 2
#define RECV_HEART_BREAK_HEADER 0

static pthread_t r_thread;   //接收消息线程
static pthread_t m_thread;   //消息处理线程
static pthread_t b_thread;   //消息缓冲区更新读写指针线程

static int sock_fd;

static pthread_mutex_t mess_buf_mutex; //消息缓冲区读写锁

static MessageBuffer mess_buf;//消息缓冲区
static char *cp_buf;
static char per_buf[MESSAGE_PROCESS_DATA_SIZE];

/**
 * 根据不同的消息标志，处理消息
 */
void MessageHandler(int sign, int total_length, char *buffer) {
    switch (sign) {
        case RECV_HEART_BREAK_HEADER:
            buffer[6] = '\0';
            debug("sign: %d, 接收线程收到%d字节心跳数据: %s\n",sign, total_length, buffer);
            break;
            //其他消息处理 ...

        default:
            break;
    }
}


//消息处理线程处理函数
void *MessageProcessFunction(void *ptr) {
    int total_length = 0;
    int sign = 0;
    char *buffer;
    while (1) {
        pthread_mutex_lock(&mess_buf_mutex);
        if(mess_buf.r_p  < mess_buf.w_p) {
            buffer = &mess_buf.buffer[mess_buf.r_p];
            sign = ntohs(*((short *) buffer));
            total_length = ntohs(*((short *) buffer + 1));
            if((total_length + mess_buf.r_p + RECEIVE_DATA_HEADER_LENGTH) <= mess_buf.w_p) {  //数据字段完整
                memcpy(per_buf, buffer + RECEIVE_DATA_HEADER_LENGTH, total_length);
                mess_buf.r_p += total_length + RECEIVE_DATA_HEADER_LENGTH;
                debug("MPF mess buf read point:%d, write point:%d\n", mess_buf.r_p, mess_buf.w_p);
                pthread_mutex_unlock(&mess_buf_mutex);
                MessageHandler(sign, total_length, per_buf);
                continue;
            } else {   //数据字段不完整
                pthread_mutex_unlock(&mess_buf_mutex);
                usleep(MESSAGE_WAIT_INTERVAL);
                continue;
            }
        }
        pthread_mutex_unlock(&mess_buf_mutex);

        usleep(MESSAGE_PROCESS_INTERVAL);
    }
}

//消息缓冲区更新读写指针x线程处理函数
void *BufferUpdateFunction(void *ptr) {
    while (1) {
        pthread_mutex_lock(&mess_buf_mutex);
        debug("BUF mess buf read point:%d, write point:%d\n", mess_buf.r_p, mess_buf.w_p);
        if(mess_buf.r_p != 0) {
            if(mess_buf.r_p == mess_buf.w_p) {
                mess_buf.r_p = 0;
                mess_buf.w_p = 0;
            } else {
                char *buffer = &mess_buf.buffer[mess_buf.r_p];
                int buffer_size = mess_buf.w_p - mess_buf.r_p;
                memcpy(cp_buf, buffer, buffer_size);
                memcpy(buffer, cp_buf, buffer_size);
                mess_buf.r_p = 0;
                mess_buf.w_p =buffer_size;
            }
        }
        debug("BUF mess buf read point:%d, write point:%d\n", mess_buf.r_p, mess_buf.w_p);
        pthread_mutex_unlock(&mess_buf_mutex);
        sleep(BUFFER_UPDATE_INTERVAL);
    }
}

//接收消息线程
void *ReceiveFunction(void *ptr) {
    debug("接收消息线程开始\n");
    int receive_sock_fd = sock_fd;
    char buffer[RECEIVE_DATA_BUFFER_SIZE + 1];

    memset(buffer, 0, RECEIVE_DATA_BUFFER_SIZE + 1);
    while (1) {
        int n = read(receive_sock_fd, buffer, RECEIVE_DATA_BUFFER_SIZE);
        if (n > 0) {
            pthread_mutex_lock(&mess_buf_mutex);
            memcpy(mess_buf.buffer, buffer, n);
            mess_buf.w_p += n;
            debug("RF mess buf read point:%d, write point:%d\n", mess_buf.r_p, mess_buf.w_p);
            pthread_mutex_unlock(&mess_buf_mutex);
        } else if (n < 0) {
            int i = errno;
            if (i != EAGAIN) {
                debug("接收线程发生错误，需重新建立！ errno is %d, %s\n", i, strerror(i));
                break;
            }
            usleep(RECEIVE_NORMAL_INTERVAL);
        } else {
            debug("服务器端关闭socket,需要重新建立连接!\n");
            break;
        }
    }
}
//初始化消息处理线程和消息缓冲区更新读写指针线程
void InitThreads() {
    if (pthread_mutex_init(&mess_buf_mutex, NULL) != 0) {
        debug("初始化缓冲区读写锁失败！\n");
        exit(0);
    } else {
        debug("初始化缓冲区读写锁成功！");
    }

    int ret;
    ret = pthread_create(&b_thread, NULL, BufferUpdateFunction, NULL);
    if (0 != ret) {
        debug("初始化更行缓冲区线程失败！\n");
        exit(0);
    }

    ret = pthread_create(&m_thread, NULL, MessageProcessFunction, NULL);
    if (0 != ret) {
        debug("初始化消息处理线程失败！\n");
        exit(0);
    }

    ret = pthread_create(&r_thread, NULL, ReceiveFunction, NULL);
    if (0 != ret) {
        debug("初始化接收消息线程失败！\n");
        exit(0);
    }

    mess_buf.r_p = 0;
    mess_buf.w_p = 0;
    mess_buf.buffer =(char *) malloc(sizeof(char) * MESSAGE_PROCESS_BUFFER_SIZE);
    cp_buf = (char *) malloc(sizeof(char) * MESSAGE_PROCESS_BUFFER_SIZE);

    if(NULL == mess_buf.buffer || NULL == cp_buf) {
        debug("申请缓冲去失败！\n");
        exit(0);
    }

    pthread_join(r_thread, NULL);
    pthread_join(b_thread, NULL);
    pthread_join(m_thread, NULL);

}


void main()
{
    int  port_no, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    port_no = PORT_NUM;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        debug("打开socket失败！\n");
        exit(0);
    } else {
        debug("打开socket成功！\n");
    }

    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        debug("没有此主机\n");
        exit(0);
    } else {
        debug("此主机存在！\n");
    }
    memset((char *) &serv_addr,0,  sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port_no);
    if (connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        debug("连接服务器失败!\n");
    } else {
        debug("连接服务器成功\n");
    }

    InitThreads();
}
