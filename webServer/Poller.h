#ifndef _POLLER_H_
#define _POLLER_H_

#include "Channel.h"
//#include "EventLoop.h"
#include <vector>
#include <map>
#include <sys/epoll.h>

class Poller
{
    typedef std::vector<Channel*> ChannelList;
    typedef std::map<int, Channel*> ChannelMap;
public:
    Poller();//EventLoop* loop
    ~Poller();

    void poll(int timeout, ChannelList &activeChannels);

    void UpdateChannel(Channel*);

    void RemoveChannel(Channel*);
private:
    int pollfd_;

    std::vector<struct epoll_event> EventList_;
    //EventLoop* ownerLoop_;
    ChannelMap ChannelMap_;
};
#endif