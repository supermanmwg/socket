
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#define DEBUG_BUILD
#ifdef DEBUG_BUILD
#define debug(fmt, ...) printf( fmt,  ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif
#define SEND_DATA_INTERVAL 1
#define PORT_NUM 10000

void SetSendData(char * random_c) {
    unsigned short sign = htons(0);
    unsigned short size = htons(5);

    int rand = 0;
    int i;
    char temp[5] = "12345";

    memcpy(random_c, &sign, 2);
    memcpy(random_c + 2, &size, 2);
    memcpy(random_c + 4, temp, 5);

}

void main(int argc, char *argv[])
{
    int sock_fd, newsock_fd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (sock_fd < 0) {
        debug("打开socket%d失败 \n", sock_fd);
    } else {
        debug("打开socket%d成功\n", sock_fd);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = PORT_NUM;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        debug("绑定socket失败\n");
    } else {
        debug("绑定socket成功\n");
    }

    listen(sock_fd,5);
    clilen = sizeof(cli_addr);
    newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &clilen);
    if(newsock_fd < 0) {
        debug("连接客户端失败!\n");
    } else {
        debug("连接客户端成功!\n");
    }
    //设置非阻塞模式
    int iMode = 1;
    ioctl(newsock_fd, FIONBIO, &iMode);
    char send_char[10];
    while(1) {
        SetSendData(send_char);
        write(newsock_fd, send_char, 9);
        sleep(SEND_DATA_INTERVAL);
    }
    close(sock_fd);
}
