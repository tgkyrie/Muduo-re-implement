#if !defined(_MUDUO_NET_SOCKETOPS_H_)
#define _MUDUO_NET_SOCKETOPS_H_
#include<netinet/in.h>
namespace muduo
{
namespace net
{
namespace sockets
{

int createNonBlockSocketOrDie(sa_family_t family);
void bindOrDie(int fd,const struct sockaddr* addr);
void listenOrDie(int fd);
int accept(int fd,struct sockaddr_in6* addr);
void close(int fd);



} // namespace socket
    
} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_SOCKETOPS_H_
