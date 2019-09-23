#include "TcpServer.h"

TcpServer::TcpServer(EventLoop* loop, int serverport, const int numthread)
    :loop_(loop),
    acceptor_(new Acceptor(loop, serverport)),
    threadpool_(new EventLoopThreadPool(loop, numthread))
{
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::newConnection,this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::start()
{
    threadpool_->start();
    acceptor_->Listen();
}

void TcpServer::newConnection(const int fd, struct sockaddr_in& clientaddr)
{
    loop_->assertInLoopThread();
    
    auto ioLoop = threadpool_->getNextLoop();
    spTcpConnection conn(new TcpConnection(ioLoop, fd, clientaddr));   //建立一个新的连接
    conn->SetMessageCallback(messageCallback_);
    conn->SetConnectionCallback(connectionCallback_);
    conn->SetSendCompleteCallback(sendCompleteCallback_);
    conn->SetCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    connections_[fd] = conn;
    ioLoop->runInLoop(std::bind(&TcpConnection::AddConnectionToLoop, conn));
}

void TcpServer::removeConnection(const spTcpConnection& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const spTcpConnection& conn)
{
    loop_->assertInLoopThread();
    connections_.erase(conn->fd());
    auto ioLoop = conn->GetLoop();
    ioLoop->addTask(std::bind(&TcpConnection::connDestroyed, conn));
    acceptor_->ReduceConn();
}