#if !defined(_MUDUO_NET_EVENTLOOPTHREADPOOL_H_)
#define _MUDUO_NET_EVENTLOOPTHREADPOOL_H_
#include"muduo/net/EventLoopThread.h"

namespace muduo
{
namespace net
{
class EventLoopThreadPool:noncopyable
{
private:
    /* data */
    EventLoop* baseLoop_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    int next_;
    int numThreads_;
    std::string name_;
public:
    EventLoopThreadPool(EventLoop* baseLoop,const std::string& name=std::string());
    ~EventLoopThreadPool()=default;
    EventLoop* getNextLoop();
    void setThreadNum(int numThreads){numThreads_=numThreads;}
    void start();
};

    
} // namespace net

    
} // namespace muduo



#endif // _MUDUO_NET_EVENTLOOPTHREADPOOL_H_
