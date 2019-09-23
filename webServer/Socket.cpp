#include "Socket.h"
#include <stdio.h> //perror
#include <stdlib.h> //exit
#include <fcntl.h>
#include <iostream>
#include <cstring>

int createSockfdNonBlock()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("createNonblockingOrDie error");
        exit(1);
    }
    setNonBlock(sockfd);
    return sockfd;
}

void setNonBlock(const int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
    {
        perror("fcntl(fd,GETFL)");
        exit(1);
    }
    if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("set O_NONBLOCK error");
        exit(1);
    }
}

void Socket::SetReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR,
        &optval, sizeof optval);
}

void Socket::BindAddress(int serverport)
{
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof serveraddr);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(serverport);
    if(bind(sockFd_, (struct sockaddr*)&serveraddr, sizeof serveraddr) == -1)
    {
        perror("error bind");
        close(sockFd_);
        exit(1);
    }
}