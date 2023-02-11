#if !defined(_MUDUO_NET_TCPSERVER_H_)
#define _MUDUO_NET_TCPSERVER_H_
#include"muduo/base/noncopyable.h"
#include"muduo/net/EventLoop.h"
#include"muduo/net/Acceptor.h"
#include"muduo/net/InetAddr.h"
#include"muduo/net/TcpConnection.h"
#include"muduo/base/Atomic.h"
#include"muduo/net/Callbacks.h"
#include"muduo/net/EventLoopThreadPool.h"
#include<map>
#include<string>

namespace muduo
{
namespace net
{


class TcpServer:noncopyable
{
public:
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop* loop,const InetAddr& addr,const string& name,bool reusePort=false);
    ~TcpServer();

    //可跨线程调用
    void start();
    void setThreadNum(int numThreads){
        threadPool_->setThreadNum(numThreads);
    }

    //accept的新连接回调，
    void newConnection(int sockfd,const InetAddr& addr);
    void setConnectionCallback(const ConnectionCallback& cb){
        connCallback_=cb;
    }
    void setMessageCallback(const MessageCallback& cb){
        messageCallback_=cb;
    }

    //删除连接回调
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);
private:
    using string=std::string;

    EventLoop* loop_;
    string IpPort_;
    const InetAddr localAddr_;
    string name_;

    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    std::map<string,TcpConnectionPtr> connMap_;
    
    //每个链接建立完成的回调
    //在TcpConnection的connectEstablished调用
    ConnectionCallback connCallback_;
    MessageCallback messageCallback_;
    AtomicIntegerT<bool> started_;
    int nextConnId_;

    

};

} // namespace net



} // namespace muduo



#endif // _MUDUO_NET_TCPSERVER_H_
