#if !defined(_MUDUO_NET_HTTP_HTTPRESPONSE_H_)
#define _MUDUO_NET_HTTP_HTTPRESPONSE_H_
#include<string>
#include<map>
#include"muduo/base/copyable.h"
#include"muduo/net/Buffer.h"
namespace muduo
{
namespace net
{
namespace http
{
class HttpResponse:copyable
{
public:
    enum HttpStatusCode{
        kUnkown,
        k200Ok=200,
        k204NoContent=204,
        k301MovedPermanently=301,
        k400BadRequest=400,
        k404NotFound=404
    };
    HttpResponse(bool closed):closeConnection_(closed){}
    ~HttpResponse()=default;
    void setHttpStatusCode(HttpStatusCode code){statusCode_=code;}
    void setStatusString(const std::string& s){statusString_=s;}
    void addHeader(const std::string& k,const std::string& v){
        header_[k]=v;
    }
    void setContentType(const std::string& contentType){ addHeader("Content-Type", contentType); }
    void setCloseConnection(bool on){
        closeConnection_=on;
    }
    void setBody(const std::string& body){body_=body;}
    void appendToBuffer(Buffer* buffer);
    bool closeConnection()const {return closeConnection_;}
private:
    /* data */
    bool closeConnection_;
    HttpStatusCode statusCode_;
    std::string statusString_;
    std::map<std::string,std::string> header_;
    std::string body_;

};

    
} // namespace http
    
} // namespace net

    
} // namespace muduo




#endif // _MUDUO_NET_HTTP_HTTPRESPONSE_H_
