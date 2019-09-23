#include "Channel.h"
#include <iostream>
#include <sys/epoll.h>
#include <assert.h>

Channel::Channel():fd_(-1), eventHandling_(false){}

Channel::~Channel(){assert(!eventHandling_);}

void Channel::HandleEvent()
{
    eventHandling_ = true;
    if(events_ & EPOLLHUP)
    {
        if(CloseHandle_) CloseHandle_();
    }
    else if(events_ & (EPOLLIN || EPOLLPRI))
    {
        if(ReadHandle_) ReadHandle_();
    }
    else if(events_ & EPOLLOUT)
    {
        if(WriteHandle_) WriteHandle_();
    }
    else
    {
        if(ErrorHandle_) ErrorHandle_();
    }
    eventHandling_ = false;
}