#if !defined(_MUDUO_NET_HTTP_HTTPSERVER_H_)
#define _MUDUO_NET_HTTP_HTTPSERVER_H_
#include"muduo/net/http/HttpContext.h"
#include"muduo/net/http/HttpRequest.h"
#include"muduo/net/http/HttpResponse.h"
#include"muduo/net/TcpServer.h"
namespace muduo
{
namespace net
{
namespace http
{
class HttpServer
{
public:
    typedef std::function<void(const HttpRequest&,HttpResponse*)> HttpCallback;
    HttpServer(EventLoop* loop,const InetAddr& addr,const std::string& name,bool reusePort=false);
    ~HttpServer()=default;
    void start();
    void setHttpCallback(const HttpCallback& cb){
        httpCallback_=cb;
    }
    void setThreadNum(int numThreads){
        server_.setThreadNum(numThreads);
    }
private:
    /* data */
    TcpServer server_;
    HttpCallback httpCallback_;
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,Buffer* buffer,Timestamp receivedTime);
    void onRequest(const TcpConnectionPtr& conn,const HttpRequest& request);
};


    
} // namespace http
    
} // namespace net
    
} // namespace muduo



#endif // _MUDUO_NET_HTTP_HTTPSERVER_H_
