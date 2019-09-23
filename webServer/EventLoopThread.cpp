#include "EventLoopThread.h"

EventLoopThread::EventLoopThread()
    :loop_(nullptr)
    {}

EventLoopThread::~EventLoopThread()
{
    std::cout << "Clean up the EventLoopThread " << std::this_thread::get_id() <<std::endl;
    loop_->quit();
    th_.join();
}

EventLoop* EventLoopThread::startLoop()
{
    th_ = std::thread(&EventLoopThread::ThreadFunc, this);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
    }
    return loop_;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    try
    {
        loop_->loop();
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr << "bad_alloc caught in EventLoopThread::ThreadFunc loop: " << ba.what() << '\n';
    }
}
