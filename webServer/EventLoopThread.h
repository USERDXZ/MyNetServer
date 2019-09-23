#ifndef _EVENTLOOP_THREAD_H_
#define _EVENTLOOP_THREAD_H_

#include "EventLoop.h"
#include <thread>
#include <mutex>
#include <condition_variable>

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void ThreadFunc();

    std::thread th_;
    EventLoop* loop_;

    std::mutex mutex_;
    std::condition_variable cond_;
};
#endif