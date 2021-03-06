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
#include <time.h>
#include "client.h"

#define __DEBUG
#ifdef __DEBUG
	#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"", __LINE__, ##__VA_ARGS__)
#else
	#define DEBUG(info)
#endif

//socket配置
#define PORT_NUMBER 60000
#define SERVER_NAME "101.200.203.52"

//定时器配置
#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1
#define TIMER_INTERVAL 5

pthread_t hbthread;  //心跳线程
pthread_t rthread;   //接收消息线程
int sockfd;	//socket 描述符
int portnum; //tcp连接端口
timer_t timerid; //定时器ID

//重启心跳和接收消息线程
void reset_tcp() {
	close_threads();
	init();
}

// 处理tcp断开，重启两个线程 
void handler(int signo) {
	if(SIGPIPE == signo) {
		reset_tcp();
	}
}

//初始化
void init() {
    int  n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
	
    // 注册SIGPIPE处理函数handler
    signal(SIGPIPE, handler);
    portnum = PORT_NUMBER;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        DEBUG("ERROR opening socket\n");
		exit(0);
	} else {
        DEBUG("RIGHT opening socket\n");
	}
    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        DEBUG("ERROR, no such host\n");
        exit(0);
    } else {
        DEBUG("RIGHT, have a  host\n");
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portnum);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        DEBUG("ERROR connecting\n");
		//exit(0);
	} else {
		DEBUG("connecting server: %s is successful\n", SERVER_NAME);
	}
	
	//建立连接
    DEBUG("Build a connection\n");
	bzero(buffer, 256);
	n = read(sockfd, buffer, 255);
	if(n > 0) {
	 	short *sign = (short *)buffer;
		printf("sign is %x \n",ntohs(sign[0]));
		char *buf = buffer + sizeof(int);
  		printf("Here is the %d message: %s\n", n - 4, buf);
	}
	
	//设置socket为非阻塞模式
	int iMode = 1;
	ioctl(sockfd, FIONBIO, &iMode);  	
}



//心跳线程处理函数
void *heart_break_function(void * ptr) {
	time_init();
}

//接收线程处理函数
void *receive_function(void * ptr) {
   char buffer[256];
 	while(1){
   		bzero(buffer, 256);
		int n = read(sockfd, buffer, 255);
		if(n > 0) {
			char *buf = buffer + sizeof(int);
  			printf("Here is the %d message: %s\n", n, buf);
		}
		usleep(1000 * 10);
 	}
}

//初始化心跳和接收消息线程
void init_threads() {
    int ret1 = pthread_create(&hbthread, NULL, heart_break_function, NULL);
	if(ret1 != 0) {
		DEBUG("heart break thread create is not ok");
		exit(0);
	}
    int ret2 = pthread_create(&rthread, NULL, receive_function, NULL);
	if(ret2 != 0) {
		DEBUG("heart break thread create is not ok");
		exit(0);
	}

    pthread_join(hbthread, NULL);
    pthread_join(rthread, NULL); 
}

//关闭心跳和接收消息线程
void close_threads() {
	//关闭心跳线程
	int hb = pthread_cancel(hbthread);
    if (0 != hb) {
		DEBUG("heart thread cancel is wrong\n");
    }
	//关闭接收消息线程
	int r = pthread_cancel(rthread);
	if(0 != r) {
		DEBUG("receive thread cancel is wrong\n");
	}
}

//定时器处理函数
void time_handler(int sig, siginfo_t *si, void *uc)
{
    if(si->si_value.sival_ptr == &timerid){
		printf("timer handler is running\n");
		time_set(timerid, 0, 0);
    }
}

//定时器设定时间函数
void time_set(timer_t timerid, int second, int nsecond){
	struct itimerspec its;
    its.it_value.tv_sec = second;
    its.it_value.tv_nsec = nsecond;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    timer_settime(timerid, 0, &its, NULL);
}

//定时器初始化
void time_init() {
    struct sigevent sev;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;

    DEBUG("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = time_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCKID, &sev, &timerid);
	time_set(timerid, TIMER_INTERVAL, 0);
}

void main() {
	init();
//	init_threads();
}
