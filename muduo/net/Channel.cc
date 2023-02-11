#include"muduo/net/Channel.h"
#include <poll.h>

#include"muduo/base/Log.h"
using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop,int fd):loop_(loop),
                    fd_(fd),
                    events_(kNoneEvent),
                    revents_(kNoneEvent),
                    index_(-1),
                    logHup_(true),
                    eventHandling_(false),
                    addedToLoop_(false)
{
}

Channel::~Channel()
{
    //LOG_DEBUG<<"Channel::dtor fd="<<fd_<<"\n";
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::update(){
    addedToLoop_=true;
    loop_->updateChannel(this);
}
void Channel::remove(){
    addedToLoop_=false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){
    handleEventWithGuard(receiveTime);
}

void Channel::handleEventWithGuard(Timestamp receiveTime){
    eventHandling_=true;

    if((revents_&POLLHUP)&&!(revents_&POLLIN)){
        if(logHup_){
            // //LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if(closeCallback_)closeCallback_();
    }
    if(revents_&POLLNVAL){
        // //LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    if(revents_ & (POLLERR | POLLNVAL)){
        if(errorCallback_)errorCallback_();
    }
    if(revents_&(POLLIN|POLLPRI|POLLRDHUP)){
        //LOG_TRACE<<"Channel::handleEvent(),beforeReadCallback,fd="<<fd_<<"\n";
        if(readCallback_)readCallback_(receiveTime);
        else{
            //LOG_TRACE<<"Channel::handleEvent(),readcb is null\n";
        }
        //LOG_TRACE<<"Channel::handleEvent(),afterReadCallback\n";
    }
    if(revents_&POLLOUT){
        if(writeCallback_)writeCallback_();
    }
    eventHandling_=false;
}