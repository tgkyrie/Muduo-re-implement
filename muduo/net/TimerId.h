#if !defined(_MUDUO_NET_TIMERID_H_)
#define _MUDUO_NET_TIMERID_H_

#include "muduo/net/Timer.h"
#include <set>
#include "muduo/base/copyable.h"
namespace muduo
{
namespace net
{
class TimerId:copyable
{
public:
    /* data */
    Timer* timer_;
    int64_t index_;
    
    friend class TimerQueue;
public:
    TimerId():index_(0),
            timer_(nullptr){}
    TimerId(Timer* timer,int64_t index):index_(index),
                    timer_(timer){}
    ~TimerId(){
    }
};
// typedef int64_t TimerId;
    
} // namespace net

} // namespace muduo



#endif // _MUDUO_NET_TIMERID_H_
