#if !defined(_MUDUO_NET_SOCKET_H_)
#define _MUDUO_NET_SOCKET_H_
#include"muduo/base/noncopyable.h"

namespace muduo
{
namespace net
{

class InetAddr;

class Socket:noncopyable
{
private:
    /* data */
    int sockfd_;
public:
    Socket(int sockfd):sockfd_(sockfd){}

    ~Socket();

    void bindInetAddr(const InetAddr& localAddr);
    void listen();
    int accept(InetAddr* peerAddr);
    void setReusePort(bool on);
    void setReuseAddr(bool on);

    void shutdownWrite();
    int fd() const {return sockfd_;}
};
    
} // namespace ne



    
} // namespace muduo



#endif // _MUDUO_NET_SOCKET_H_
