#if !defined(_MUDUO_NET_CALLBACKS_)
#define _MUDUO_NET_CALLBACKS_

#include<functional>
#include<memory>

namespace muduo
{
namespace net
{
class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&,Buffer*,Timestamp)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (TcpConnectionPtr&)> WriteCompleteCallback;

void defaultConnCallback(const TcpConnectionPtr& conn);
} // namespace net
    

} // namespace muduo



#endif // _MUDUO_NET_CALLBACKS_
