#ifndef _TIMER_H_
#define _TIMER_H_

#include "TcpConnection.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Socket.h"
#include "Channel.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <unordered_set>

class Entry
{
    typedef std::weak_ptr<TcpConnection> wpConn;
public:
    explicit Entry(std::shared_ptr<TcpConnection> spConn);
    ~Entry();
private:
    wpConn wpConn_;
};

class TimerManager
{
    typedef std::shared_ptr<Entry> spEntry;
    typedef std::unordered_set<spEntry> SpEntrySet;
    typedef std::vector<SpEntrySet> EntryWheel;
public:
    TimerManager();
    ~TimerManager();

    void start();

    void AddToTimer(spEntry& newEntry, size_t timeout);
    
    void AddInLoop(spEntry& newEntry, size_t timeout);

    //void Adjust();

private:
    void handleRead();

    EventLoopThread loopThread_;

    EventLoop* loop_;

    Socket timerfd_;

    Channel timerChannel_;

    EntryWheel entryWheel_;

    size_t nextIndex_;
};
#endif