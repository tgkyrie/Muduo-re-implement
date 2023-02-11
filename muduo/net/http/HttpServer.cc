#include"muduo/net/http/HttpServer.h"
#include"muduo/base/Log.h"
using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::http;

using namespace std::placeholders;
HttpServer::HttpServer(EventLoop* loop,const InetAddr& addr,const std::string& name,bool reusePort):
server_(loop,addr,name,reusePort)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage,this,_1,_2,_3));

}
void HttpServer::start(){
    server_.start();
}
void HttpServer::onConnection(const TcpConnectionPtr& conn){
    conn->setContext(HttpContext());
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,Buffer* buffer,Timestamp receivedTime){
    // LOG_TRACE<<"HttpServer::onMessage()\n";
    Any& any=conn->getContext();
    HttpContext& context=any.cast<HttpContext>();
    assert(context.gotAll()!=999);
    HttpResponse response(true);
    
    bool noErr=context.parseRequest(buffer,receivedTime);
    if(!noErr){
        // conn->send()
        // std::cout<<"HttpServer::onMessage() parseRequestErr\n";
        conn->shutdown();
    }
    else if(context.gotAll()){
        onRequest(conn,context.request());
        context.reset();
    }
}
void HttpServer::onRequest(const TcpConnectionPtr& conn,const HttpRequest& request){
    //LOG_TRACE<<"HttpServer::onRequest()\n";
    const string& connection = request.getHeader("connection");
    bool close=(connection=="closed")||(request.getVersion() == HttpRequest::kHttp10 && connection != "keep-alive");
    HttpResponse response(close);
    httpCallback_(request,&response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if(response.closeConnection()){
        conn->shutdown();
    }
}