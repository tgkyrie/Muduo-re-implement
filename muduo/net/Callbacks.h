#if !defined(_MUDUO_NET_CALLBACKS_)
#define _MUDUO_NET_CALLBACKS_

#include<functional>
#include<memory>

#include "muduo/base/Timestamp.h"

namespace muduo
{
namespace net
{
class TcpConnection;
class Buffer;
class File;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::shared_ptr<File> FilePtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&,Buffer*,Timestamp)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
void defaultConnCallback(const TcpConnectionPtr& conn);
} // namespace net
    

} // namespace muduo



#endif // _MUDUO_NET_CALLBACKS_
