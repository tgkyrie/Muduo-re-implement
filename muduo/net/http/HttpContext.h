#if !defined(_MUDUO_NET_HTTP_HTTPCONTEXT_H_)
#define _MUDUO_NET_HTTP_HTTPCONTEXT_H_
#include"muduo/net/Buffer.h"
#include"muduo/base/Timestamp.h"
#include"muduo/net/http/HttpRequest.h"
#include"muduo/base/copyable.h"
namespace muduo
{
namespace net
{
namespace http
{
class HttpContext:copyable
{

public:
    enum HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };
    HttpContext():state_(kExpectRequestLine){}
    ~HttpContext()=default;
    bool parseRequest(Buffer* buffer,Timestamp receiveTime);
    bool gotAll()const {return state_==kGotAll;}
    HttpRequest& request(){
        return httpRequest_;
    }
    const HttpRequest& request()const{
        return httpRequest_;
    }
    void reset(){
        state_=kExpectRequestLine;
        HttpRequest dummy;
        httpRequest_.swap(dummy);
    }
private:
    /* data */
    bool parseRequestBody(Buffer* buffer);
    bool parseRequestLine(const char* begin,const char* end);
    HttpRequest httpRequest_;
    HttpRequestParseState state_;
    
    //if get complete request return 1,else return 0,if error return -1


};




} // namespace http

    
} // namespace ne

    
} // namespace muduo



#endif // _MUDUO_NET_HTTP_HTTPCONTEXT_H_
