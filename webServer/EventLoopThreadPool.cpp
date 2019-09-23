#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const int numThread)
    :baseLoop_(baseLoop),
    numThread_(numThread),
    next_(0)
    {
        for(int i = 0; i < numThread_; ++i)
            threads_.push_back(spThread(new EventLoopThread));
    }

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start()
{
    for(int i = 0; i < numThread_; ++i)
        loops_.push_back(threads_[i]->startLoop());
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();

    EventLoop* loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_++];
        if (next_ >= numThread_) next_ = 0;
    }
    return loop;
}