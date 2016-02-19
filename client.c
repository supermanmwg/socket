#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//hanlde SIGPIPE signal
void handler(int s) {
    printf("Caught SIGPIPE\n");
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

   // ignore the SIGPIPE ERROR
    signal(SIGPIPE, handler);
    portno = 10000;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
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
    printf("Please enter the message\n ");
    bzero(buffer,256);
    bcopy("hello world", buffer, 11);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) {
         printf("ERROR writing to socket");
    }
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         printf("ERROR reading from socket");
    printf("%s\n",buffer);
	sleep(1);
    }

    close(sockfd);
    return 0;
}
