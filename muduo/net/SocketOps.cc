#include"muduo/net/SocketOps.h"
#include<sys/socket.h>
#include<errno.h>
#include<unistd.h>
#include"muduo/base/Log.h"

using namespace muduo;
using namespace muduo::net;

int sockets::createNonBlockSocketOrDie(sa_family_t family){
    int ret=::socket(family,SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK,IPPROTO_TCP);
    if (ret < 0)
    {
        LOG_DEBUG<<"create socket failed"<<std::endl;
        // LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    return ret;
}

void sockets::bindOrDie(int fd,const struct sockaddr* addr){
    int ret=::bind(fd,addr,sizeof(struct sockaddr_in6));
    if (ret < 0)
    {
        LOG_DEBUG<<"bindOrDie failed"<<std::endl;
        // LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockets::listenOrDie(int fd){
    int ret=::listen(fd,32768);
    if (ret < 0)
    {
        LOG_DEBUG<<"listenOrDie failed"<<std::endl;
        // LOG_SYSFATAL << "sockets::listenOrDie";
    }
}
int sockets::accept(int fd,struct sockaddr_in6* addr){
    socklen_t addrLen=sizeof(sockaddr_in6);
    int ret=::accept4(fd,(struct sockaddr*)addr,&addrLen,SOCK_CLOEXEC|SOCK_NONBLOCK);
    int savedErrno=errno;
    if(ret<0){
        // LOG_SYSERR << "Socket::accept";
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            // LOG_FATAL << "unexpected error of ::accept " << savedErrno;
            break;
        default:
            // LOG_FATAL << "unknown error of ::accept " << savedErrno;
            break;
        }
    }
    else{
        // LOG_DEBUG<<"accept a conn"<<std::endl;
    }
    return ret;
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        // LOG_SYSERR << "sockets::close";
    }
}