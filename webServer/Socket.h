#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //close
#include <iostream>

int createSockfdNonBlock();

void setNonBlock(const int fd);

class Socket
{
public:
    explicit Socket(int sockFd)
        :sockFd_(sockFd){};

    ~Socket()
    {
        std::cout <<  "closed fd:" << sockFd_ << std::endl;
        close(sockFd_);
    }

    int fd() const
    {return sockFd_;}

    void SetReuseAddr(bool on);

    void BindAddress(int serverport);

private:
    const int sockFd_;
};
#endif