#ifndef _MUDUO_NET_EVENTLOOP_H_
#define _MUDUO_NET_EVENTLOOP_H_
#include <atomic>
#include <functional>
#include <vector>
#include <memory>

#include <boost/any.hpp>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
// #include "muduo/net/Poller.h"
// #include "muduo/net/Channel.h"
#include "muduo/base/Atomic.h"
#include "muduo/net/Callbacks.h"
#include"muduo/net/TimerId.h"
// #include "muduo/net/TimerQueue.h"

namespace muduo
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop:noncopyable
{

public:
    typedef std::function<void()> Functor;

    EventLoop(/* args */);
    ~EventLoop();
    void loop();
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);


    void abortNotInLoopThread();
    bool isInLoopThread(){
        return threadId_==t_currentThread.tid();
    }

    //推断当前线程是IO线程
    void assertInLoopThread(){
        if(!isInLoopThread()){
            abortNotInLoopThread();
        }
    }

    //跨线程调用
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    TimerId runAt(Timestamp time,TimerCallback cb);
    TimerId runAfter(double seconds,TimerCallback cb);
    TimerId runEvery(double seconds,TimerCallback cb);
    void cancel(TimerId timerId);
    void handleRead();
    void doPendingFunctors();

    void wakeup();
private:
    /* data */
    bool looping_;          //是否在loop中
    AtomicIntegerT<bool> quit_;
    bool eventHandling_;    //是否在处理事件中

    int64_t iteration_;
    const pid_t threadId_; 
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    typedef std::vector<Channel*> ChannelList;
    
    ChannelList activeChannels_;
    Channel* currentActiveChannel_; 

    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    MutexLock pendingFunctorsMutex_;
    std::vector<Functor> pendingFunctors_;

};



    
} // namespace net
    
} // namespace muduo





#endif // _MUDUO_NET_EVENTLOOP_H_
