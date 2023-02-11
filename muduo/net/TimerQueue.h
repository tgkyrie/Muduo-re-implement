#if !defined(_MUDUO_NET_TIMERQUEUE_H_)
#define _MUDUO_NET_TIMERQUEUE_H_

// #include"muduo/net/EventLoop.h"
// #include "muduo/net/TimerId.h"

// #include<unordered_map>
// #include<queue>
// #include<list>
#include<vector>
#include<set>
#include<map>
#include<memory>
#include"muduo/base/Timestamp.h"
#include"muduo/base/noncopyable.h"
#include"muduo/net/Callbacks.h"
#include"muduo/net/Channel.h"
#include"muduo/base/MinHeap.h"
namespace muduo
{
namespace net
{
class EventLoop;
class Timer;


class TimerQueue: noncopyable
{
public:
    class Entry{
    public:
        std::pair<Timestamp,Timer*> entry_;
    public:
        Entry():entry_(0,0){}
        Entry(Timestamp expiration,Timer* timer):entry_(expiration,timer){}
        ~Entry()=default;
        // void setMinHeapIndex(ssize_t idx){
        //     entry_.second->setHeapIndex(idx);
        // }
        bool operator<(const Entry& that) const{
            return entry_<that.entry_;
        }
        bool operator>(const Entry& that) const{
            return entry_>that.entry_;
        }
        const Timestamp expiration()const{return entry_.first;}
        Timer*  timer()const{return entry_.second;}
    };
    // typedef std::pair<Timestamp,Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<std::unique_ptr<Timer>,size_t> MapValue;
    typedef std::map<int64_t,std::unique_ptr<Timer>> TimerIdMap;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();
    TimerId addTimer(TimerCallback cb,Timestamp expiration,double interval);
    void cancel(TimerId timerId);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    // int64_t addTimerInLoop(std::unique_ptr<Timer>&& timer);
    TimerId addTimerInLoop(Timer* timer);
    // TimerList::iterator insert(Timer* timer,bool& earliestChanged);
    size_t insert(Timer* timer ,bool& earliestChanged);
    std::vector<Entry> getExpired(Timestamp now);
    void reset(std::vector<Entry>& expired, Timestamp now);
    
private:
    /* data */
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    MinHeap<Entry> timerQueue_;
    // TimerIdMap timerMap_;
    // TimerList timerList_;
    // TimerIdMap timers_;
public:

};


    
} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_TIMERQUEUE_H_
