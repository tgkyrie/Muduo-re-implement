#if !defined(_MUDUO_NET_INETADDR_H_)
#define _MUDUO_NET_INETADDR_H_
#include<stdint.h>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>


namespace muduo
{
namespace net
{

class InetAddr
{
private:
    /* data */
    union 
    {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
    
    
public:
    InetAddr()=default;
    explicit InetAddr(std::string Ip,uint16_t port,bool ipv6=false);
    explicit InetAddr(uint16_t port,bool loopBackOnly=false,bool ipv6=false);
    explicit InetAddr(sockaddr_in& addr){
        addr_=addr;
    }
    explicit InetAddr(sockaddr_in6& addr6){
        addr6_=addr6;
    }
    ~InetAddr()=default;
    sa_family_t family() const{return addr_.sin_family;}
    std::string Ip() const;
    std::string IpPort() const;
    uint16_t port() const;
    const struct sockaddr* getSockAddr() const { return (const struct sockaddr*)&addr6_; }
    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }
};




} // namespace net
    
} // namespace muduo




#endif // _MUDUO_NET_INETADDR_H_
