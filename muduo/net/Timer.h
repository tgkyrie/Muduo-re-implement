#if !defined(_MUDUO_NET_TIMER_H_)
#define _MUDUO_NET_TIMER_H_

#include "muduo/base/Timestamp.h"
#include "muduo/base/Atomic.h"
#include "muduo/net/Callbacks.h"

#include<boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class Timer:boost::noncopyable
{
private:
    /* data */
    const TimerCallback timerCallback_;
    Timestamp expiration_;
    const double interval_;
    bool repeat_;
    bool canceled_;
    const int64_t index_;
    ssize_t heapIndex_;

    static AtomicInt64 s_numCreated_;
    friend class TimerQueue;
public:
    Timer(TimerCallback timerCallback,Timestamp expiration,double interval):
            timerCallback_(timerCallback),
            expiration_(expiration),
            interval_(interval),
            repeat_(interval>0),
            canceled_(false),
            index_(s_numCreated_.incrementAndGet()),
            heapIndex_(-1){}
    void run() const{
        timerCallback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t index() const { return index_; }
    void setHeapIndex(ssize_t idx){
        heapIndex_=idx;
    }
    ssize_t heapIndex()const{return heapIndex_;}

    void restart(Timestamp now);
    // TimerCallback cb(){return timerCallback_;}
    static int64_t numCreated() { return s_numCreated_.get(); }

    const bool canceled()const{return canceled_;}
    void cancel(){
        canceled_=true;
        repeat_=false;
    }
};



} // namespace net
    

} // namespace muduo



#endif // _MUDUO_NET_TIMER_H_
