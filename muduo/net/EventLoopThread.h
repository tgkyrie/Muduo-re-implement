#if !defined(_MUDUO_NET_EVENTLOOPTHREAD_H_)
#define _MUDUO_NET_EVENTLOOPTHREAD_H_
#include"muduo/net/EventLoop.h"
#include"muduo/base/Thread.h"
#include"muduo/base/CountDownLatch.h"


namespace muduo
{
namespace net
{

class EventLoopThread:noncopyable
{
private:
    /* data */
    string name_;
    EventLoop* loop_;
    Thread thread_;
    CountDownLatch latch_;
    void loop();
public:
    EventLoopThread(const std::string& name=std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
    EventLoop* getLoop()const {return loop_;}

};


} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_EVENTLOOPTHREAD_H_
