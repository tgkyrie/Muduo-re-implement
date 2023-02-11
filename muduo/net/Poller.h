#ifndef _MUDUO_NET_POLLER_H_
#define _MUDUO_NET_POLLER_H_

#include <map>
#include <vector>

#include "muduo/base/Timestamp.h"
#include "muduo/net/EventLoop.h"
#include"muduo/base/noncopyable.h"

namespace muduo{

namespace net
{

class Channel;
class Poller:noncopyable
{
private:
    /* data */
    EventLoop* ownerLoop_;
protected:
    typedef std::unordered_map<int,Channel*> ChannelMap;
    ChannelMap channels_;


public:
    Poller(EventLoop* loop);
    virtual ~Poller();
    typedef std::vector<Channel*> ChannelList;
    virtual Timestamp poll(int timeOutMs,ChannelList* activeChannels)=0;
    virtual void updateChannel(Channel* channel)=0;
    virtual void removeChannel(Channel* channel)=0;

    //该函数只能在所属的 evenloop 中调用
    //功能就是查找当前 IO 线程是否否在 channel 对象
    virtual bool hasChannel(Channel* channel) const;

    void assertInLoopThread() const{
        ownerLoop_->assertInLoopThread();
    }

    
};





} // namespace net

} // namespace muduo

#endif