#include "Acceptor.h"

const int MAXCONN = 20000;

Acceptor::Acceptor(EventLoop* loop, const int serverport)
    :connections_(0),
    loop_(loop),
    acceptSocket_(createSockfdNonBlock()),
    listening_(false)
    {
        acceptSocket_.SetReuseAddr(true);
        acceptSocket_.BindAddress(serverport);
        
        acceptChannel_.SetFd(acceptSocket_.fd());
        acceptChannel_.SetReadHandle(std::bind(&Acceptor::handleRead, this));
    }

void Acceptor::Listen()
{
    loop_->assertInLoopThread();
    listening_ = true;

    if(listen(acceptSocket_.fd(), 8192) < 0)
    {
        perror("error listen");
        close(acceptSocket_.fd());
        exit(1);
    }
    acceptChannel_.SetEvents(EPOLLIN | EPOLLET);
    loop_->UpdateChannelToPoller(&acceptChannel_);
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    assert(listening_);
    sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    int connfd;
    while((connfd = accept(acceptSocket_.fd(), (struct sockaddr*)&clientaddr, &addrlen)) > 0)
    {
        if(connections_ > MAXCONN)
        {
            close(connfd);
            break;
        }
        connections_++;
        setNonBlock(connfd);
        newConnectionCb_(connfd, clientaddr);
    }
}