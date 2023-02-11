#include "muduo/net/poller/EpollPoller.h"
#include "muduo/net/Channel.h"
#include"muduo/base/Log.h"
#include <sys/epoll.h>
using namespace muduo;
using namespace muduo::net;

namespace 
{
const int kNew=-1;
const int kAdded=1;
const int kDeleted=2;
} // namespace 

EpollPoller::EpollPoller(EventLoop* loop):Poller(loop),
                epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        // LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}
EpollPoller::~EpollPoller(){
    ::close(epollfd_);
}


Timestamp EpollPoller::poll(int timeOutMs,ChannelList* activeChannels){
    // LOG_TRACE << "fd total count " << channels_.size();
    int numEvents=epoll_wait(epollfd_,&(*events_.begin()),events_.size(),timeOutMs);
    int savedErrno=errno;
    Timestamp now(Timestamp::now());
    if(numEvents>0){
        fillActiveChannels(numEvents,activeChannels);
        if(static_cast<size_t>(numEvents)==events_.size()){
            events_.resize(2*events_.size());
        }
    }
    else if(numEvents==0){
        // LOG_TRACE<<"nothing happen";
    }
    else {
        if(savedErrno!=EINTR){
            errno=savedErrno;
            // LOG_SYSERR<<"EpollPoller::poll()";
        }
    }
    return now;
}

void EpollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels){
    for(int i=0;i<numEvents;i++){
        
        Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
        //for debug
        //LOG_DEBUG<<"poll channel fd="<<channel->fd()<<"\n";
        //end 
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel){
      Poller::assertInLoopThread();
    const int index = channel->index();
    int fd = channel->fd();
    // LOG_TRACE << "fd = " << channel->fd()
    //     << " events = " << channel->events() << " index = " << index;
    if(index==::kNew||index==::kDeleted){
        // epoll_ctl(EPOLL_CTL_ADD)
        if(index==::kNew){
            assert(channels_.find(fd) == channels_.end());
            channels_[fd]=channel;
        }
        else{
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(::kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else {
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        if(channel->isNoneEvent()){
            channel->setIndex(kDeleted);
            update(EPOLL_CTL_DEL,channel);
        }
        else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel){
    int fd=channel->fd();
    int index=channel->index();
    //   //LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    
    size_t n=channels_.erase(fd);
    //LOG_DEBUG<<"EpollPoller::removeChannel()::erase fd,n="+std::to_string(fd)+"\n";
    (void)n;
    assert(n==1);

    if(index==kAdded){
        //LOG_DEBUG<<"EpollPoller::removeChannel(),epoll_del\n";
        update(EPOLL_CTL_DEL,channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::update(int operation,Channel* channel){
    struct epoll_event event;
    memset(&event,0,sizeof event);
    event.events=channel->events();
    event.data.ptr=channel;
    int fd=channel->fd();
    if(operation==EPOLL_CTL_DEL){
        //LOG_INFO<<"EpollPoller::update() epoll_del fd= "<<channel->fd()<<"\n";
    }
    if(::epoll_ctl(epollfd_,operation,fd,&event)<0){
        if (operation == EPOLL_CTL_DEL)
        {
            //LOG_TRACE<<"error in epollctl delete\n";
            // //LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
        else
        {
            //LOG_TRACE<<"error in epollctl add or modify\n";
            // LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}