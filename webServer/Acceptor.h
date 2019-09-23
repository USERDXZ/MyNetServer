#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

class Acceptor
{
    typedef std::function<void (const int fd,
        struct sockaddr_in &clientaddr)> NewConnectionCallback;
public:
    Acceptor(EventLoop* loop, const int serverport);
    ~Acceptor(){}

    void SetNewConnectionCallback(const NewConnectionCallback& cb)
    {newConnectionCb_ = cb;}
    
    void Listen();

    void ReduceConn()
    {connections_--;}

private:
    void handleRead();

    int connections_;

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCb_;
    bool listening_;
};
#endif