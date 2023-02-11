#ifndef _MUDUO_NET_CHANNEL_H_
#define _MUDUO_NET_CHANNEL_H_

#include<muduo/net/EventLoop.h>
#include "muduo/base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{
    
namespace net
{
class Channel:noncopyable
{

public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;
    Channel(EventLoop* loop,int fd);
    ~Channel();

    void setReadCallback(const ReadEventCallback& cb){
        // readCallback_=std::move(cb);
        readCallback_=cb;
    }
    void setWriteCallback(EventCallback cb){
        writeCallback_=std::move(cb);
    }
    void setErrorCallback(EventCallback cb){
        errorCallback_=std::move(cb);
    }
    void setCloseCallback(EventCallback cb){
        closeCallback_=std::move(cb);
    }
    void update();
    void remove();

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    
    void handleEvent(Timestamp receiveTime);
    void handleEventWithGuard(Timestamp receiveTime);

    int fd() const { return fd_; }
    int events() const { return events_; }
    int index() const {return index_;}
    void setIndex(int index){index_=index;}
    void setRevents(int revents){revents_=revents;}
    EventLoop* ownerLoop(){return loop_;}

private:
    /* data */
    EventLoop* loop_;   //所属loop
    int fd_;            //文件描述符
    int events_;         //关注的事件
    int revents_;        //触发的事件
    int index_;     
    bool logHup_;

    bool eventHandling_;
    bool addedToLoop_;

    //三种事件类型
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    //四种回调指针
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
    


};

    


} // namespace net

} // namespace muduo


#endif