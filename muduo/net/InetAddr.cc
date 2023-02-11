#include "muduo/net/InetAddr.h"
#include<string.h>
#include<arpa/inet.h>
#include"muduo/base/Log.h"
using namespace muduo;
using namespace muduo::net;

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
InetAddr::InetAddr(std::string Ip,uint16_t port,bool ipv6){
    if(ipv6){
        memset(&addr6_,0,sizeof addr6_);
        addr6_.sin6_family=AF_INET6;
        addr6_.sin6_port=htons(port);
        inet_pton(AF_INET6,Ip.c_str(),&addr6_.sin6_addr);
    }
    else{
        memset(&addr_,0,sizeof addr_);
        addr_.sin_family=AF_INET;
        addr_.sin_port=htons(port);
        inet_pton(AF_INET,Ip.c_str(),&addr_.sin_port);
    }
}

InetAddr::InetAddr(uint16_t port,bool loopBackOnly,bool ipv6){
    if(ipv6){
        memset(&addr6_,0,sizeof addr6_);
        addr6_.sin6_family=AF_INET6;
        addr6_.sin6_port=htons(port);
        addr6_.sin6_addr=loopBackOnly ? in6addr_loopback : in6addr_any;
    }
    else{
        memset(&addr_,0,sizeof addr_);
        addr_.sin_family=AF_INET;
        addr_.sin_port=htons(port);
        // inet_pton(AF_INET,Ip.c_str(),&addr_.sin_port);
        in_addr_t ip=loopBackOnly?kInaddrLoopback:kInaddrAny;
        addr_.sin_addr.s_addr=htonl(ip);
    
    }
}

std::string InetAddr::Ip() const{
    char ip[64]="";
    if(addr_.sin_family==AF_INET){
        inet_ntop(addr_.sin_family,&addr_.sin_addr,ip,sizeof(ip));
    }
    else if(addr_.sin_family==AF_INET6){
        inet_ntop(addr6_.sin6_family,&addr6_.sin6_addr,ip,sizeof(ip));
    }
    //LOG_DEBUG<<"InetAddr::Ip(),ip="<<ip<<"\n";
    return ip;
}
std::string InetAddr::IpPort() const{
    return Ip()+":"+std::to_string(port());
}
uint16_t InetAddr::port() const{
    if(addr_.sin_family==AF_INET){
        return ntohs(addr_.sin_port);
    }
    else if(addr_.sin_family==AF_INET6){
        return ntohs(addr6_.sin6_port);
    }
    else {
        return 0;
    }
}