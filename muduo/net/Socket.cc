#include "muduo/net/Socket.h"
#include "muduo/net/SocketOps.h"
#include "muduo/net/InetAddr.h"
#include "muduo/base/Log.h"
#include<unistd.h>
#include<string.h>
using namespace muduo;
using namespace muduo::net;


Socket::~Socket(){
    sockets::close(sockfd_);
}

void Socket::bindInetAddr(const InetAddr& localAddr){

    sockets::bindOrDie(sockfd_,localAddr.getSockAddr());
}
void Socket::listen(){
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddr* peerAddr){
    sockaddr_in6 addr;
    memset(&addr,0,sizeof addr);
    int conn=sockets::accept(sockfd_,&addr);
    peerAddr->setSockAddrInet6(addr);
    return conn;
}

void Socket::shutdownWrite(){
    int ret=shutdown(sockfd_,SHUT_WR);
    if(ret<0){
        //LOG_DEBUG<<"Socket::shutdownWrite() ERR shutdownWrite"<<std::endl;
    }
    //LOG_TRACE<<"Socket::shutdownWrite() shutdownWrite finish"<<std::endl;
}

void Socket::setReusePort(bool on){
    int val=on;
    int ret=setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&val,sizeof(val));
    if(ret<0){
    //
    }
}

void Socket::setReuseAddr(bool on){
    int val=on;
    int ret=setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
    if(ret<0){
    //
    }
}