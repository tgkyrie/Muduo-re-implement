#if !defined(_MUDUO_NET_ACCEPTOR_H_)
#define _MUDUO_NET_ACCEPTOR_H_

#include"muduo/net/Socket.h"
#include"muduo/net/Channel.h"
#include"muduo/net/EventLoop.h"
#include"muduo/net/InetAddr.h"


namespace muduo
{

namespace net
{


class Acceptor:noncopyable
{   
public:
    typedef std::function<void(int sockfd,const InetAddr& addr)> NewConnectionCallback;
    Acceptor(EventLoop* loop,const InetAddr& addr,bool reusePort);
    ~Acceptor();
    void listen();
    void handleRead();
    void setNewConnectionCallback(NewConnectionCallback cb){newConnectionCallback_=std::move(cb);}
private:
    /* data */
    static const int acceptPerEvent=16;
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    int idlefd_;
    NewConnectionCallback newConnectionCallback_;

};




} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_ACCEPTOR_H_
