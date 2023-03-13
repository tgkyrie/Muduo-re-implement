#include"muduo/net/TcpServer.h"
#include<iostream>
using namespace muduo;
using namespace muduo::net;
#include"muduo/base/Log.h"


TcpServer::TcpServer(EventLoop* loop,const InetAddr& addr,const string& name,bool reusePort):
loop_(loop),
localAddr_(addr),
IpPort_(addr.IpPort()),
name_(name),
acceptor_(new Acceptor(loop,addr,reusePort)),
threadPool_(new EventLoopThreadPool(loop,name)),
connCallback_(defaultConnCallback),
nextConnId_(1),
started_()
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2)
    );
}
TcpServer::~TcpServer(){
    loop_->assertInLoopThread();
    for (auto& item : connMap_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->loop()->runInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start(){
    if(started_.getAndSet(1)==0){
        std::cout<<"run server at"+IpPort_<<std::endl;
        threadPool_->start();
        loop_->runInLoop(
            std::bind(&Acceptor::listen,acceptor_.get())
        );
    }
}

//when acceptor.accept return ,run this
void TcpServer::newConnection(int sockfd,const InetAddr& addr){
    // LOG_INFO<<++connectNum<<"\n";
    loop_->assertInLoopThread();
    char buf[64];
    snprintf(buf,sizeof buf,"-%s#%d",addr.IpPort().c_str(),nextConnId_);
    nextConnId_++;
    string name=name_+buf;
    //LOG_INFO << "TcpServer::newConnection [" << name_
        // << "] - new connection [" << name
        // << "] from " << addr.IpPort()<<std::endl;
    EventLoop* ioloop=threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioloop,name,sockfd,localAddr_,addr));
    connMap_[name]=conn;//加入链接表
    conn->setConnectionCallback(connCallback_);//设置链接建立回调
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    //链接建立完成
    ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn.get()));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){
    //LOG_TRACE<<"TcpServer::removeConnetion()\n";
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
    loop_->assertInLoopThread();
    // //LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
    //     << "] - connection " << conn->name();
    //LOG_DEBUG<<"erase conn\n";
    connMap_.erase(conn->name());
    //LOG_DEBUG<<"after erase\n";
    EventLoop* ioloop=conn->loop();
    ioloop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn)
    );
  
}