#include "Poller.h"
#include <iostream>
#include <unistd.h> //close
#include <stdio.h>  //perror
#include <stdlib.h> //exit
#include <assert.h>

const int EVENTNUM = 4096;

Poller::Poller()//EventLoop* loop)
    ://ownerLoop_(loop),
    EventList_(EVENTNUM)
{
    if((pollfd_ = epoll_create(256)) == -1)
    {
        perror("epoll create error");
        exit(1);
    }
    std::cout << "epoll_create:" << pollfd_ << std::endl;
}

Poller::~Poller()
{
    close(pollfd_);
}

void Poller::poll(int timeout, ChannelList& activeChannels)
{
    int numsEvents = epoll_wait(pollfd_, &*EventList_.begin(), (int)EventList_.size(), timeout);
    //assert (numsEvents != -1);
    for(int i = 0; i < numsEvents; ++i)
    {
        Channel* pChannel = (Channel*)EventList_[i].data.ptr;
        int fd = pChannel->GetFd();
        auto iter = ChannelMap_.find(fd);

        assert(iter != ChannelMap_.end());
        assert(iter->second == pChannel);

        pChannel->SetEvents(EventList_[i].events);
        activeChannels.push_back(pChannel);
    }
    if(numsEvents == (int)EventList_.size())
    {
        std::cout << "resize Eventlist" << std::endl;
        EventList_.resize(numsEvents * 2);
    }
}

void Poller::UpdateChannel(Channel* pChannel)
{
    //ownerLoop_->assertInLoopThread();

    int fd = pChannel->GetFd();
    struct epoll_event ev;
    ev.events = pChannel->GetEvents();
    ev.data.ptr = pChannel;
    
    auto iter = ChannelMap_.find(fd);
    if(iter == ChannelMap_.end())
    {
        ChannelMap_[fd] = pChannel;
        if(epoll_ctl(pollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("epoll add error");
            exit(1);
        }
    }
    else
    {
        if(epoll_ctl(pollfd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            perror("epoll mod error");
            exit(1);
        }
    }
    
}

void Poller::RemoveChannel(Channel* pChannel)
{
    //ownerLoop_->assertInLoopThread();

    int fd = pChannel->GetFd();
    struct epoll_event ev;
    ev.events = pChannel->GetEvents();
    ev.data.ptr = pChannel;

    ChannelMap_.erase(fd);

    if(epoll_ctl(pollfd_, EPOLL_CTL_DEL, fd, &ev) == -1)
    {
        perror("epoll del error");
        exit(1);
    }
}