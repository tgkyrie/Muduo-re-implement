#include"muduo/net/EventLoopThreadPool.h"
using namespace muduo;
using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,const std::string& name):
baseLoop_(baseLoop),
numThreads_(0),
next_(0),
name_(name)
{   
}

void EventLoopThreadPool::start(){
    baseLoop_->assertInLoopThread();
    for(int i=0;i<numThreads_;i++){
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* thread=new EventLoopThread(buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(thread));
        loops_.push_back(thread->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    baseLoop_->assertInLoopThread();
    EventLoop* loop=baseLoop_;
    if(!loops_.empty()){
        loop=loops_[next_++];
        if(next_>=loops_.size()){
            next_=0;
        }
    }
    return loop;
}