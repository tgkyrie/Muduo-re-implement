#include "muduo/net/TimerQueue.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Timer.h"
#include <sys/timerfd.h>
#include<iostream>
using namespace muduo;
using namespace muduo::net;

namespace 
{

int createTimerfd(){
    int fd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);
    if(fd<0){
        // LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return fd;
}
void readTimerfd(int timerfd,Timestamp now){
    uint64_t howmany;
    size_t n=read(timerfd,&howmany,sizeof howmany);
    if(n!=howmany){
        // LOG_ERROR<<"timerQueueHandle read "<<n<<"bytes but expected 8";
    }
}


struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                        - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd,Timestamp expiration){
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue,0,sizeof(struct itimerspec));
    newValue.it_value=howMuchTimeFromNow(expiration);
    int ret=timerfd_settime(timerfd,0,&newValue,&oldValue);
    if(ret<0){
        // LOG_SYSERR<<"timerfd_settime";
    }
}



} // namespace 


TimerQueue::TimerQueue(EventLoop* loop):loop_(loop),
                        timerfd_(createTimerfd()),
                        timerfdChannel_(loop,timerfd_)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                        Timestamp expiration,
                        double interval)
{
    // Timer* timer=new Timer(std::move(cb),expiration,interval);
    Timer* timer=new Timer(std::move(cb),expiration,interval);
    // timerMap_.insert(timer->index(),std::move(timer));
    return addTimerInLoop(timer);
}



// int64_t TimerQueue::addTimerInLoop(std::unique_ptr<Timer>&& timer)
// {
//     Timer* rawTimer=timer.get();
//     bool earliestChanged;

//     int timerId=timer->index();
//     TimerList::iterator it=insert(timer.get(),earliestChanged);
//     int64_t index=timer->index();
//     MapValue value(std::move(timer),it);
//     timers_.insert(std::pair<int64_t,MapValue>(index,std::move(value)));
//     if(earliestChanged){
//         resetTimerfd(timerfd_,rawTimer->expiration());
//     }
//     return timerId;
// }
TimerId TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged=false;
    Timestamp when=timer->expiration();
    Entry entry(when,timer);
    if(timerQueue_.empty()||when<timerQueue_.top().expiration()){
        earliestChanged=true;
    }
    timerQueue_.push(std::move(entry));
    if(earliestChanged){
        resetTimerfd(timerfd_,when);
    }
    int64_t index=timer->index();
    return TimerId(timer,index);

}
void TimerQueue::handleRead(){
    Timestamp now=Timestamp::now();
    readTimerfd(timerfd_,now);
    std::vector<Entry> expired=getExpired(now);
    // std::vector<TimerCallback> pendingFunctors;
    for(auto& it:expired){
        if(!it.timer()->canceled())it.timer()->run();
    }
    reset(expired,now);
}
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now){
    std::vector<Entry> ret;
    timerQueue_.assertOrder();
    Entry lowerBoundEntry=Entry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    while(!timerQueue_.empty()&&timerQueue_.top()<lowerBoundEntry){
        ret.push_back(timerQueue_.top());
        timerQueue_.pop();
    }
    return ret;
}


void TimerQueue::reset(std::vector<Entry>& expired, Timestamp now){
    Timestamp nextExpiration;
    for(Entry& it:expired){
        if(it.timer()->repeat()&&!it.timer()->canceled()){
            it.timer()->restart(now);
            timerQueue_.push(Entry(it.timer()->expiration(),it.timer()));
        }
        else{
            delete it.timer();
        }
    }
    timerQueue_.assertOrder();
    if(!timerQueue_.empty()){
        nextExpiration=timerQueue_.top().timer()->expiration();

    }
    if(nextExpiration.valid()){
        resetTimerfd(timerfd_,nextExpiration);
    }
}

void TimerQueue::cancel(TimerId timerId){
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop,this,timerId)
    );
}

void TimerQueue::cancelInLoop(TimerId timerId){
    timerId.timer_->cancel();
}


// TimerQueue::TimerList::iterator
// TimerQueue::insert(Timer* timer,bool& earliestChanged){
//     earliestChanged=false;
//     Timestamp when=timer->expiration();

//     TimerList::iterator it=timerList_.begin();
//     if(it==timerList_.end()||when<it->first){
//         earliestChanged=true;
//     }
//     // timerList_.insert(std::move(Entry(when,std::move(timer))));
//     std::pair<TimerList::iterator, bool> result
//         =timerList_.insert(Entry(when,timer->index()));
    
//     int64_t index=timer->index();
//     TimerIdMap::iterator itTimer=timers_.find(index);
//     if(itTimer!=timers_.end()){
//         //超时事件重新插入,维护Map中的迭代器
//         // timers_.find(index)->second.second=result.first; 
//         timers_[index].second=result.first;  
//     }
//     return result.first;
// }

// std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now){
//     std::vector<Entry> expired;
//     Entry lowerBoundEntry=Entry(now,static_cast<int64_t>(INT64_MAX));
//     TimerList::iterator it=timerList_.lower_bound(lowerBoundEntry);

//     std::copy(timerList_.begin(),it,std::back_inserter(expired));
//     timerList_.erase(timerList_.begin(),it);
//     for(auto& entry:expired){
//         timers_[entry.second].second=timerList_.end();
//     }
//     return expired;
// }

// void TimerQueue::handleRead(){
//     Timestamp now=Timestamp::now();
//     readTimerfd(timerfd_,now);
//     std::vector<Entry> expired=getExpired(now);
//     // std::vector<TimerCallback> pendingFunctors;
//     for(auto& it:expired){
//         int index=it.second;
//         TimerIdMap:: iterator mapIt=timers_.find(index);
//         if(mapIt!=timers_.end()){
//             mapIt->second.first->run();
//         }
//         // timers_[index].second=timerList_.end();
//         // // pendingFunctors.emplace_back(timers_[index].first->timerCallback_);
//         // timers_[index].first->run();
//         //不能在这里执行回调
//         //因为超时Timer还没有reset
//         //执行的回调可能会有cancel timer的操作
//         //这时要cancel的timer在unorderedMap但不再set中,不能set.erase
//     }
//     reset(expired,now);
//     //FIXME:被取消的任务可能再被执行一次
//     //比如在一个任务a，在第10秒取消任务b,而任务b在第10.1秒执行,
//     //假设此时是10.2秒，a，b超时，保存了ab回调函数，执行a后b也执行了
//     // for(auto& cb:pendingFunctors){
//     //     cb();        
//     // }
// }

// void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now){
//     Timestamp nextExpiration;
//     for(const Entry& it:expired){
//         int index=it.second;
//         // if(timers_.find(index))
//         TimerIdMap::iterator mapIt=timers_.find(index);
//         if(mapIt!=timers_.end()){
//             std::unique_ptr<Timer>& timer=mapIt->second.first;
//             if(timer->repeat()){
//                 timer->restart(now);
//                 bool changed;
//                 insert(timer.get(),changed);
//                 // timer->run();
//             }
//             else{
//                 // TimerCallback cb=std::move(timer->timerCallback_);
//                 // std::cout<<"timer:"<<timer->index()<<" end"<<std::endl;
//                 timers_.erase(index);
//             }
//         }
//     }
//     if(!timerList_.empty()){
//         nextExpiration=timers_[timerList_.begin()->second].first
//                         ->expiration();
//     }
//     if(nextExpiration.valid()){
//         resetTimerfd(timerfd_,nextExpiration);
//     }
// }

// void TimerQueue::cancel(int64_t timerId){
//     if(timers_.find(timerId)!=timers_.end()){
//         Timestamp when=timers_[timerId].first->expiration();
//         TimerList::iterator it=timers_[timerId].second;
//         int64_t index=timers_[timerId].first->index();
//         // assert(timerList_.find(Entry(when,index))!=timerList_.end());
//         // assert(timerList_.find(Entry(when,index))==it);
//         // std::cout<<"before erase"<<std::endl;
//         // try
//         // {
//         if(it!=timerList_.end()){
//             timerList_.erase(it);
//         }
//         // }
//         // catch(const std::exception& e)
//         // {
//         //     std::cerr << e.what() << '\n';
//         //     std::cout<<"erase failed"<<std::endl;
//         // }
        
//         // std::cout<<"after erase"<<std::endl;
//         timers_.erase(timerId);
//         // std::cout<<"find"<<std::endl;
//     }
//     // std::cout<<"end cancel"<<std::endl;
    
// }


//与muduo源码比较
//使用unique_ptr管理Timer对象
//用set<Timestamp,index> + unodered_map<index,unique_ptr<Timer>>

//                      本代码          muduo           
//新增timer           O(lgn)+O(1)     O(lgn)+O(lgn)
//                红黑树插入+哈希表插入  两次红黑树插入

//取消timer           均摊O(1)           O(lgn)
//               哈希表查找红黑树迭代器  红黑树查找删除
//                     红黑树删除       
//k个超时事件处理         均摊O(k)        均摊O(k)
//内存