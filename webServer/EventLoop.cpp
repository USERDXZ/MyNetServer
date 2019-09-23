#include "EventLoop.h"
#include <sys/eventfd.h>
#include <unistd.h> //read

__thread EventLoop* t_loopInThisThread = 0;
const int EpollTimesMs = 1000;

//参照muduo，实现跨线程唤醒, 文件被设置成 O_CLOEXEC，创建子进程 (fork) 时不继承父进程的文件描述符。,
//文件被设置成 O_NONBLOCK，执行 read / write 操作时，不会阻塞。
int CreateEventFd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        perror("Failed in eventfd");
        close(evtfd);
        exit(1);
    }
    return evtfd;
}

EventLoop::EventLoop():
    looping_(false), 
    quit_(false), 
    callingAddTask_(false),
    threadId_(std::this_thread::get_id()),
    poller_(),//this
    wakeupFd_(CreateEventFd())
{
    if(t_loopInThisThread)
    {
        std::cout << "error:another loop exists in this thread" << std::endl;
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupchannel_.SetFd(wakeupFd_);
    wakeupchannel_.SetEvents(EPOLLIN | EPOLLET);
    wakeupchannel_.SetReadHandle(std::bind(&EventLoop::handleRead, this));
    UpdateChannelToPoller(&wakeupchannel_);
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::abortNotInLoopThread()
{
    std::cout << "error:loop not in this thread;" << std::endl;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activeChannels_.clear();
        poller_.poll(EpollTimesMs, activeChannels_);
        for(auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->HandleEvent();
        }
        ExecuteTask();
    }
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::runInLoop(const Functor& cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        addTask(cb);
    }
    
}

void EventLoop::addTask(const Functor& cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        FunctorList_.push_back(cb);
    }
    if(!isInLoopThread() || callingAddTask_)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, (char*)(&one), sizeof one);
}

void EventLoop::ExecuteTask()
{
    std::vector<Functor> FunctorList;
    callingAddTask_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        FunctorList.swap(FunctorList_);
    }
    for(auto func:FunctorList)
    {
        func();
    }
    callingAddTask_ = false;
}