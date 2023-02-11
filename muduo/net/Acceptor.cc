#include"muduo/net/Acceptor.h"
#include"muduo/net/SocketOps.h"
#include"muduo/base/Log.h"
#include<sys/types.h>
#include<fcntl.h>
// #include"muduo/base/LOG.h"
using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop,const InetAddr& addr,bool reusePort):
loop_(loop),
acceptSocket_(sockets::createNonBlockSocketOrDie(addr.family())),
acceptChannel_(loop,acceptSocket_.fd()),
idlefd_(open("/dev/null",O_CLOEXEC|O_RDONLY))
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptSocket_.bindInetAddr(addr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor(){
    sockets::close(idlefd_);
}

void Acceptor::listen(){
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead(){
    InetAddr peerAddr;
    //LOG_TRACE<<"acceptor::handleRead()\n";
    int connfd;
    int i=0;
    while((connfd=acceptSocket_.accept(&peerAddr))>=0&&i<acceptPerEvent){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd,peerAddr);
        }
        else{
            //LOG_TRACE<<"acceptor::handleRead() no newconncb\n";
            close(connfd);
        }
        i++;
    }
    if(connfd<0){
        if(errno==EMFILE){
            LOG_INFO << "in Acceptor::handleRead ERR connfd<0 errno="<<errno<<"\n";
            close(idlefd_);
            idlefd_=accept(acceptSocket_.fd(),nullptr,nullptr);
            close(idlefd_);
        }
    }
    // int connfd=acceptSocket_.accept(&peerAddr);
    // if(connfd>=0){
    //     if(newConnectionCallback_){
    //         newConnectionCallback_(connfd,peerAddr);
    //     }
    //     else{
    //         //LOG_TRACE<<"acceptor::handleRead() no newconncb\n";
    //         close(connfd);
    //     }
    // }
    // else{
    //     if(errno==EMFILE){
    //         LOG_INFO << "in Acceptor::handleRead ERR connfd<0 errno="<<errno<<"\n";
    //         close(idlefd_);
    //         idlefd_=accept(acceptSocket_.fd(),nullptr,nullptr);
    //         close(idlefd_);
    //     }
    // }

}