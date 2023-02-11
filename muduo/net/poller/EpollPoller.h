#ifndef _MUDUO_NET_EPOLLPOLLER_
#define _MUDUO_NET_EPOLLPOLLER_

// #include"muduo/net/EventLoop.h"
#include"muduo/net/Poller.h"

struct epoll_event;

namespace muduo
{

namespace net
{

class EpollPoller:public Poller
{
private:
    /* data */
    static const int kInitEventListSize=16;
    int epollfd_;
    typedef std::vector<struct epoll_event> EventList;
    EventList events_;
    
    void update(int operation,Channel* channel);
    
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller();
    Timestamp poll(int timeOutMs,ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
    void fillActiveChannels(int numEvents,ChannelList* activeChannels);
};
 
} // namespace net


} // namespace muduo






#endif