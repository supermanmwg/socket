#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>

//init the thread
pthread_t thread1;
int is_cancel = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//hanlde SIGPIPE signal
void handler(int s) {
    printf("Caught SIGPIPE\n");
	if(is_cancel == 0) {
		printf("thread cancel is begin\n");
		int s = pthread_cancel(thread1);
        if (s != 0) {
		   	printf("thread cancel is wrong\n");
		}
		is_cancel = 1;
	}
}

/*
	thread function
**/
void *print_message_function(void *ptr) {
     char *message;
     message = (char *) ptr;
	 while(1) {
		sleep(1);
     	printf("%s \n", message);
 	 }
}

#define SERVER_NAME "localhost"
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
	
    char *message1 = "Thread 1";
    int iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
	if(iret1 != 0) {
		printf("thread create is not ok");
	}

    // ignore the SIGPIPE ERROR
    signal(SIGPIPE, handler);
    portno = 10000;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    while(1) {
		sleep(1);
    	bzero(buffer,256);
    	bcopy("hello world", buffer, 11);
    	n = write(sockfd,buffer,strlen(buffer));
    	if (n < 0) {
         	printf("ERROR writing to socket");
    	}
    	bzero(buffer,256);
    	n = read(sockfd,buffer,255);
    	if (n < 0) { 
         	printf("ERROR reading from socket");
		} else {
     		printf("%s\n",buffer);
    	}
    }

    close(sockfd);
	
    pthread_join( thread1, NULL);
    return 0;
}
