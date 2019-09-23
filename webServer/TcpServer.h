#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include <string>
#include <functional>
#include <map>

class TcpServer
{
    typedef std::shared_ptr<TcpConnection> spTcpConnection;
    
    typedef std::function<void(const spTcpConnection&, std::string&)> MessageCallback;
    typedef std::function<void(const spTcpConnection&)> Callback;

    typedef std::map<int, spTcpConnection> connectionMap;
public:
    TcpServer(EventLoop*, int serverport, const int numthread);
    ~TcpServer(){std::cout << connections_.size() << " connetions need clean" << std::endl;}

    void start();
    //void stop();

    void SetMessageCallback(const MessageCallback& mcb)
    {messageCallback_ = mcb;}

    void SetConnectionCallback(const Callback& cb)
    {connectionCallback_ = cb;}

    void SetsendCompleteCallback(const Callback& cb)
    {sendCompleteCallback_ = cb;}
    
private:
    void newConnection(const int connSock, struct sockaddr_in& clientaddr);

    void removeConnection(const spTcpConnection& conn);

    void removeConnectionInLoop(const spTcpConnection& conn);

    EventLoop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadpool_;

    connectionMap connections_;

    Callback connectionCallback_;
    Callback sendCompleteCallback_;
    MessageCallback messageCallback_;
};

#endif