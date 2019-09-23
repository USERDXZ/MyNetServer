#include "Timer.h"
#include <sys/time.h>
#include <assert.h>
#include <string.h>

static int createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC,
                            TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        perror("create timerfd error");
        close(timerfd);
        exit(1);
    }
    return timerfd;
}

Entry::Entry(std::shared_ptr<TcpConnection> spConn)
    :wpConn_(spConn)
    {}
    
Entry::~Entry()
{
    auto spConn = wpConn_.lock();
    if(spConn)
    {
        spConn->shutDown();
    }
}

TimerManager::TimerManager()
    :loop_(nullptr),
    timerfd_(createTimerfd()),
    entryWheel_(60),
    nextIndex_(0)
    {
        timerChannel_.SetFd(timerfd_.fd());
        timerChannel_.SetReadHandle(std::bind(&TimerManager::handleRead, this));
        timerChannel_.SetEvents(EPOLLIN | EPOLLET);
    }

TimerManager::~TimerManager()
{

}

void TimerManager::start()
{
    loop_ = loopThread_.startLoop();

    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));
    /*
    struct timespec now;
    bzero(&now, sizeof(now));
    if(clock_gettime(timerfd_.fd(), &now) < 0)
    {
        std::cout << "clock_gettime error" << std::endl;
        exit(1);
    }
    */
    newValue.it_value.tv_sec = 1;
    newValue.it_value.tv_nsec = 0;
    newValue.it_interval.tv_sec = 1;
    newValue.it_interval.tv_nsec = 0;
    timerfd_settime(timerfd_.fd(), 0, &newValue, NULL);

    loop_->addTask(std::bind(&EventLoop::UpdateChannelToPoller, loop_, &timerChannel_));
}


void TimerManager::handleRead()
{
    uint64_t exp;
    ssize_t s = read(timerfd_.fd(), &exp, sizeof(uint64_t));
    //std::cout << "1 sec past, nextIndex:" << nextIndex_ << std::endl;
    SpEntrySet newSet;
    entryWheel_[nextIndex_++].swap(newSet);
    if(nextIndex_ >= 60) nextIndex_ = 0;
}

void TimerManager::AddToTimer(spEntry& newEntry, size_t timeout)
{
    loop_->runInLoop(std::bind(&TimerManager::AddInLoop, this, newEntry, timeout));
}

void TimerManager::AddInLoop(spEntry& newEntry,size_t timeout)
{
    loop_->assertInLoopThread();
    int index = nextIndex_ + timeout;
    if(index >= 60) index %= 60;
    entryWheel_[index].insert(newEntry);
}

/* 
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) +timeout;
}
*/