#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

class Channel
{
    typedef std::function<void()> CallBack;
public:
    Channel(/* args */);
    ~Channel();

    void SetFd(int fd)
    {fd_ = fd;}
    int GetFd() const
    {return fd_;}

    void SetEvents(uint32_t events)
    {events_ = events;}

    uint32_t GetEvents()
    {return events_;}

    void HandleEvent();

    void SetWriteHandle(const CallBack& cb)
    {WriteHandle_ = cb;}

    void SetReadHandle(const CallBack& cb)
    {ReadHandle_ = cb;}

    void SetErrorHandle(const CallBack& cb)
    {ErrorHandle_ = cb;}

    void SetCloseHandle(const CallBack& cb)
    {CloseHandle_ = cb;}
private:
    int fd_;
    uint32_t events_;

    bool eventHandling_;

    CallBack WriteHandle_;
    CallBack ReadHandle_;
    CallBack ErrorHandle_;
    CallBack CloseHandle_;
};
#endif