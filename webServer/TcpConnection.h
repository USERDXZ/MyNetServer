#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <netinet/in.h>
#include <memory>
#include <any>

class Entry;

ssize_t readn(int fd, std::string &inBuffer);

ssize_t writen(int fd, std::string &sbuff);

class TcpConnection:public std::enable_shared_from_this<TcpConnection>
{
    typedef std::shared_ptr<TcpConnection> spTcpConnection;
    typedef std::function<void(const spTcpConnection&)> Callback;
    typedef std::function<void(const spTcpConnection&, std::string&)> MessageCallBack;
public:
    TcpConnection(EventLoop* loop, int fd, struct sockaddr_in &clientaddr);
    ~TcpConnection();

    int fd() const
    {return Socket_->fd();}

    EventLoop* GetLoop() const
    {return loop_;}

    void AddConnectionToLoop();

    void send(const std::string&);

    void sendInLoop();
    
    void shutDown();

    void shutDownInLoop();

    void connDestroyed();

    void SetMessageCallback(const MessageCallBack& cb)
    {messageCallback_ = cb;}

    void SetSendCompleteCallback(const Callback& cb)
    {sendcompleteCallback_ = cb;}

    void SetCloseCallback(const Callback& cb)
    {closeCallback_ = cb;}

    void SetConnectionCallback(const Callback& cb)
    {connectionCallback_ = cb;}

    void setWpEntry(std::weak_ptr<Entry>& wpEntry)
    {wpEntry_ = wpEntry;}

    std::weak_ptr<Entry> getWpEntry() const
    {return wpEntry_;}
    
private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    bool closed_;
    bool halfclosed_; 

    std::unique_ptr<Socket> Socket_;

    EventLoop* loop_;

    struct sockaddr_in clientaddr_;

    Channel Channel_;

    std::weak_ptr<Entry> wpEntry_;
    
    std::string bufferin_;
    std::string bufferout_;

    MessageCallBack messageCallback_;
    Callback sendcompleteCallback_;
    Callback closeCallback_;
    Callback connectionCallback_;
};
#endif