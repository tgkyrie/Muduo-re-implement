#include"muduo/net/EventLoop.h"
#include "muduo/net/poller/EpollPoller.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Poller.h"
#include "muduo/net/TimerQueue.h"
#include"muduo/base/Log.h"
#include<signal.h>

#include<sys/eventfd.h>
using namespace muduo;
using namespace muduo::net;

namespace
{
thread_local EventLoop* tLoopInThisThread = 0;    //
const int kPollTimeMs = 10000;

int createEventfd(){
    int fd=eventfd(0,EFD_CLOEXEC|EFD_NONBLOCK);
    if(fd<0){
        // //LOG_SYSERR<<"failed in create eventfd";
    }
    return fd;
}
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // //LOG_TRACE << "Ignore SIGPIPE";
  }
};

IgnoreSigPipe gIgnoreSigPipe;

} // namespace


EventLoop::EventLoop():looping_(false),
                    eventHandling_(false),
                    iteration_(0),
                    threadId_(t_currentThread.tid()),
                    poller_(new EpollPoller(this)),
                    currentActiveChannel_(nullptr),
                    timerQueue_(new TimerQueue(this)),
                    wakeupFd_(createEventfd()),
                    wakeupChannel_(new Channel(this,wakeupFd_))

{
    if(tLoopInThisThread){
        // //LOG_FATAL << "Another EventLoop " << t_loopInThisThread
        //       << " exists in this thread " << threadId_;
    }
    else {
        tLoopInThisThread=this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop(){
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    tLoopInThisThread=nullptr;
}

void EventLoop::updateChannel(Channel* channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
    //LOG_DEBUG<<"EventLoop::removeChannel() remove "+std::to_string(channel->fd())+"\n";
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::loop(){
    assert(!looping_);
    assertInLoopThread();
    looping_=true;
    quit_.getAndSet(false);
    eventHandling_=false;


    while (1)
    {
        /* code */
        iteration_++;
        activeChannels_.clear();
        pollReturnTime_=poller_->poll(kPollTimeMs,&activeChannels_);
        // if (Logger::logLevel() <= Logger::TRACE){
        //     printActiveChannels();
        // }
        eventHandling_=true;
        for(Channel* channel:activeChannels_){
            currentActiveChannel_=channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_=nullptr;
        eventHandling_=false;
        doPendingFunctors();
    }
    // ////LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_=false;
}

void EventLoop::quit(){
    quit_.getAndSet(true);
}

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb){
    {
        MutexLockGuard lock(pendingFunctorsMutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread()){
        wakeup();
    }
}

void EventLoop::cancel(TimerId timerId){
    timerQueue_->cancel(timerId);
}

void EventLoop::wakeup(){
    uint64_t buf=1;
    size_t n=write(wakeupFd_,&buf,sizeof buf);
    if(n!=sizeof buf){
        // ////LOG_ERROR<<"EvenLoop::wakeup write "<<n<< "bytes instead of 8";
    }
}

void EventLoop::handleRead(){
    uint64_t buf;
    size_t n=read(wakeupFd_,&buf,sizeof buf);
    if (n != sizeof buf)
    {
        // ////LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    {
        MutexLockGuard lock(pendingFunctorsMutex_);
        functors.swap(pendingFunctors_);
    }
    for(auto& cb:functors){
        cb();
    }
}

TimerId EventLoop::runAt(Timestamp time,TimerCallback cb){
    return timerQueue_->addTimer(std::move(cb),time,0.0);
}

TimerId EventLoop::runAfter(double seconds,TimerCallback cb){
    return timerQueue_->addTimer(std::move(cb),addTime(Timestamp::now(),seconds),0.0);
}

TimerId EventLoop::runEvery(double seconds,TimerCallback cb){
    return timerQueue_->addTimer(std::move(cb),addTime(Timestamp::now(),seconds),seconds);
}

void EventLoop::abortNotInLoopThread(){
    //   //LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
    //         << " was created in threadId_ = " << threadId_
    //         << ", current thread id = " <<  t_currentThread.tid();
}