#include"muduo/net/http/HttpResponse.h"
using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::http;

void HttpResponse::appendToBuffer(Buffer* buffer){
    char buf[32];
    snprintf(buf,sizeof buf,"HTTP/1.1 %d ",statusCode_);
    buffer->append(buf,strlen(buf));
    buffer->append(statusString_.c_str(),statusString_.size());
    buffer->append("\r\n");

    if(closeConnection_){
        buffer->append("Connection: close\r\n");
    }
    else{
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        buffer->append(buf);
        buffer->append("Connection: Keep-Alive\r\n");
    }

    for (const auto& header : header_)
    {
        buffer->append(header.first);
        buffer->append(": ");
        buffer->append(header.second);
        buffer->append("\r\n");
    }

  buffer->append("\r\n");
  buffer->append(body_);
}