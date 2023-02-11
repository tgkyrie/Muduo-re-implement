#if !defined(_MUDUO_NET_TCPCONNECTION_H_)
#define _MUDUO_NET_TCPCONNECTION_H_
#include"muduo/net/InetAddr.h"
#include"muduo/net/Socket.h"
#include"muduo/net/Channel.h"
#include"muduo/net/Callbacks.h"
#include"muduo/base/noncopyable.h"
#include"muduo/net/Buffer.h"
#include"muduo/base/Any.h"
#include<string>
#include<memory>

namespace muduo
{
namespace net
{
class EventLoop;
class TcpConnection:noncopyable,
                    public std::enable_shared_from_this<TcpConnection>
{
public:
    using string=std::string;
    TcpConnection(EventLoop* loop,const string& name,int sockfd,const InetAddr& localAddr,const InetAddr& peerAddr);
    ~TcpConnection();
    void connectEstablished();
    void connectDestroyed();

    void setConnectionCallback(const ConnectionCallback& cb){
        connectionCallback_=cb;
    }
    void setCloseCallback(const CloseCallback& cb){
        closeCallback_=cb;
    }
    void setMessageCallback(const MessageCallback& cb){
        messageCallback_=cb;
    }
    string name()const {return name_;}
    EventLoop* loop()const {return loop_;}
    const InetAddr localAddr()const{return localAddr_;}
    const InetAddr peerAddr()const{return peerAddr_;}
    bool connected(){return state_==kConnected;}
    void send(Buffer* buf);
    void send(const char* s);
    void send(const std::string& s);
    void send(const void* buf,size_t len);

    void shutdown();

    Any& getContext(){
        return context_;
    }
    template <typename T>
    void setContext(T&& context){
        //LOG_TRACE<<"TcpConnection::setContext()\n";
        context_=std::forward<T>(context);
    }

private:
    /* data */
    enum State{kConnecting,kConnected,kDisConnecting,kDisConnected};
    State state_;
    EventLoop* loop_;
    bool reading_;
    string name_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> sockChannel_;

    const InetAddr localAddr_;
    const InetAddr peerAddr_;

    //链接建立完成或删除完成回调
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    Any context_;

    //事件处理函数
    void handleRead(Timestamp receiveTime);
    void handleError();
    void handleClose();
    void handleWrite();

    void sendInLoopAsString(const std::string s);
    void sendInLoop(const void* buf,size_t len);
    void shutdownInLoop();

};

// void defaultConnCallback(const TcpConnectionPtr& conn);


} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_TCPCONNECTION_H_
