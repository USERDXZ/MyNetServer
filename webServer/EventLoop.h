#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include "Channel.h"
#include "Poller.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <assert.h>
#include <memory>
#include <functional>

class EventLoop
{
    typedef std::vector<Channel*> ChannelList;
    typedef std::function<void()> Functor;
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void UpdateChannelToPoller(Channel* pChannel)
    {
        assertInLoopThread();
        poller_.UpdateChannel(pChannel);
    }

    void RemoveChannelToPoller(Channel* pChannel)
    {
        assertInLoopThread();
        poller_.RemoveChannel(pChannel);
    }

    void quit();

    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {return threadId_ == std::this_thread::get_id();}

    void runInLoop(const Functor& cb);

    void addTask(const Functor& cb);

    void wakeup();

private:
    void abortNotInLoopThread();
    void handleRead();
    void ExecuteTask();

    bool looping_;
    bool quit_;
    bool callingAddTask_;

    std::thread::id threadId_;
    Poller poller_;
    ChannelList activeChannels_;

    std::mutex mutex_;
    int wakeupFd_;
    Channel wakeupchannel_;
    std::vector<Functor> FunctorList_;
};
#endif