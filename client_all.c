#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#define __DEBUG
#ifdef __DEBUG
	#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
	#define DEBUG(info)
#endif

void closeThreads();
void init();

#define PORT_NUMBER 10000
#define SERVER_NAME "localhost"

pthread_t hbThread;  //心跳线程
pthread_t rThread;   //接收消息线程
int sockfd;	//socket 描述符
int portNum;

//重启心跳和接收消息线程
void resetTcp() {
	closeThreads();
	init();
}

// 处理tcp断开，重启两个线程 
void handler() {
	resetTcp();
}


//初始化
void init() {
    int  n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
	
    // 注册SIGPIPE处理函数handler
    signal(SIGPIPE, handler);
    portNum = PORT_NUMBER;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        DEBUG("ERROR opening socket\n");
    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        DEBUG("ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portNum);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        DEBUG("ERROR connecting\n");
	
	//设置socket为非阻塞模式
	int iMode = 1;
	ioctl(sockfd, FIONBIO, &iMode);  	
}

//初始化心跳和接收消息线程
void initThread(int sockfd) {

}

//关闭心跳和接收消息线程
void closeThreads() {
	//关闭心跳线程
	int hb = pthread_cancel(hbThread);
    if (0 != hb) {
		DEBUG("heart thread cancel is wrong\n");
    }
	//关闭接收消息线程
	int r = pthread_cancel(rThread);
	if(0 != r) {
		DEBUG("receive thread cancel is wrong\n");
	}
}
