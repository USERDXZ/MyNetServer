#ifndef _EVENTLOOPTHREADPOOL_H_
#define _EVENTLOOPTHREADPOOL_H_

#include "EventLoopThread.h"

class EventLoopThreadPool
{
    typedef std::unique_ptr<EventLoopThread> spThread; 
public:
    explicit EventLoopThreadPool(EventLoop* baseLoop, const int numThread);
    ~EventLoopThreadPool();

    void start();

    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;

    int numThread_;
    int next_;

    std::vector<spThread> threads_;
    std::vector<EventLoop*> loops_;
};
#endif