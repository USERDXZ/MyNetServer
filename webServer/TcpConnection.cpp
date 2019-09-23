#include "TcpConnection.h"
const int  MAX_BUFF = 4096;

TcpConnection::TcpConnection(EventLoop* loop, int fd, struct sockaddr_in& addr)
    :closed_(true),
    halfclosed_(false),
    loop_(loop),
    Socket_(new Socket(fd)),
    clientaddr_(addr)
{
    Socket_->SetReuseAddr(true);
    Channel_.SetFd(fd);
    Channel_.SetReadHandle(std::bind(&TcpConnection::handleRead, this));
    Channel_.SetWriteHandle(std::bind(&TcpConnection::handleWrite, this));
    Channel_.SetCloseHandle(std::bind(&TcpConnection::handleClose, this));
    Channel_.SetErrorHandle(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::AddConnectionToLoop()
{
    loop_->assertInLoopThread();
    closed_ = false;
    Channel_.SetEvents(EPOLLIN);
    loop_->UpdateChannelToPoller(&Channel_);

    std::cout << "new connection from IP: " << inet_ntoa(clientaddr_.sin_addr)
        << ":" << ntohs(clientaddr_.sin_port) << " fd:" << Socket_->fd() << std::endl;
    if(connectionCallback_)
        connectionCallback_(shared_from_this());
}

void TcpConnection::send(const std::string& message)
{
    bufferout_ += message;
    loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this));
}

void TcpConnection::sendInLoop()
{
    if(closed_) return ;
    ssize_t n = writen(Socket_->fd(), bufferout_);
    if(n > 0)
    {
        auto events = Channel_.GetEvents();
        if(bufferout_.size() > 0)
        {
            Channel_.SetEvents(events | EPOLLOUT);
            loop_->UpdateChannelToPoller(&Channel_);
        }
        else if(bufferout_.size() == 0)
        {
            if(sendcompleteCallback_) 
                loop_->addTask(std::bind(sendcompleteCallback_, shared_from_this()));
            if(halfclosed_)
                handleClose();
        }
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        handleError();
    }
    
}

void TcpConnection::shutDown()
{
    std::cout << "conn shutDown!" << std::endl;
    loop_->runInLoop(std::bind(&TcpConnection::shutDownInLoop, this));
}

void TcpConnection::shutDownInLoop()
{
    //handleClose();
    if(closed_) return;
    loop_->assertInLoopThread();
    //Channel_.SetEvents(0);
    //loop_->UpdateChannelToPoller(&Channel_);
    closed_ = true;
    closeCallback_(shared_from_this());
}

void TcpConnection::connDestroyed()
{
    loop_->assertInLoopThread();
    loop_->RemoveChannelToPoller(&Channel_);
}

void TcpConnection::handleRead()
{
    if(closed_) return ;
    ssize_t n = readn(Socket_->fd(), bufferin_);
    if(n > 0){
        messageCallback_(shared_from_this(), bufferin_);
    }
    else if(n == 0){
        handleClose();
    }
    else
    {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if(closed_) return ;
    ssize_t n = writen(Socket_->fd(), bufferout_);
    if(n > 0)
    {
        auto events = Channel_.GetEvents();
        if(bufferout_.size() > 0)
        {
            Channel_.SetEvents(events | EPOLLOUT);
            loop_->UpdateChannelToPoller(&Channel_);
        }
        else if(bufferout_.size() == 0)
        {
            Channel_.SetEvents(events & ~(EPOLLOUT));
            if(sendcompleteCallback_) 
                loop_->addTask(std::bind(sendcompleteCallback_, shared_from_this()));
            if(halfclosed_)
                handleClose();
        }
        
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        handleError();
    }
}

void TcpConnection::handleError()
{
    if(closed_) return ;
    //perror("TcpConnection::handleError");
    handleClose();
}

//对端关闭连接
void TcpConnection::handleClose()
{
    if(closed_) return ;
    loop_->assertInLoopThread();
    if(bufferout_.size() > 0 || bufferin_.size() > 0)  //优雅关闭
    {
        halfclosed_ = true; std::cout << "set halfclosed" << std::endl;
        if(bufferin_.size() > 0)
        {
            messageCallback_(shared_from_this(), bufferin_);
        }
    }
    else
    {
        //Channel_.SetEvents(0);
        //loop_->UpdateChannelToPoller(&Channel_);
        closed_ = true;                     //后面剩下资源释放和removeChannel
        closeCallback_(shared_from_this()); //TcpServer才能移除this
    }
}

ssize_t readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {
                return readSum;
            }  
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            break;
        }
        readSum += nread;
        inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}

ssize_t writen(int fd, std::string &sbuff)
{
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}
