#include"muduo/net/http/HttpContext.h"
// #include"muduo/base/LOG.h"
// #include"muduo/base/Log.h"

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::http;

bool HttpContext::parseRequestLine(const char* begin,const char* end){
    // LOG_TRACE<<"HttpContext::parseRequestLine()\n";
    bool ok=false;
    const char* start=begin;
    const char* space=std::find(start,end,' ');
    if(space!=end&&httpRequest_.setMethod(start,space)){//request.setMethod
        start=space+1;
        space=std::find(start,end,' ');
        if(space!=end){
            const char* question=std::find(start,space,'?');
            if(question==space){
                //setPath
                httpRequest_.setPath(start,space);
            }
            else{
                //setPath
                httpRequest_.setPath(start,question);
                httpRequest_.setQuery(question+1,space);
                //setQuery
            }
            start=space+1;
            ok=end-start==8&&std::equal(start,end-1,"HTTP/1.");
            if(ok){
                char v=*(end-1);
                if(v=='0'){
                    //setVersion
                    httpRequest_.setVersion(HttpRequest::kHttp10);
                }
                else if(v=='1'){
                    httpRequest_.setVersion(HttpRequest::kHttp11);
                }
                else{
                    httpRequest_.setVersion(HttpRequest::kUnkown);
                    ok=false;
                }
            }
        }
        
    }
    return ok;
}
bool isNum(const std::string& s){
    for(auto&c:s){
        if(c<'0'||c>'9')return false;
    }
    return true;
}
bool HttpContext::parseRequestBody(Buffer* buffer){
    // LOG_TRACE<<"HttpContext::parseRequestBody()\n";
    bool ok=true;
    std::string lengthStr(httpRequest_.getHeader("content-length"));
    if(isNum(lengthStr)){
        int length=atoi(lengthStr.c_str());
        // LOG_TRACE<<"Length:"<<length<<"\n";
        if(length<buffer->readableBytes()){
            return true;
        }
        else if(length==buffer->readableBytes()){
            httpRequest_.setBody(buffer->peek(),buffer->peek()+buffer->readableBytes());
            buffer->retrieveAll();
            state_=kGotAll;
            return true;
        }
        else{
            LOG_TRACE<<"HttpContext::parseRequestBody() failed\n";
            return false;
        }
    }
    else return false;
    return true; //unreach
}
//HeaderList -> k : v CRLF HeaderList | CRLF
bool HttpContext::parseRequest(Buffer* buffer,Timestamp receiveTime){
    //LOG_TRACE<<"HttpContext::parseRequest()\n";
    bool ok=true;
    bool hasMore=true;
    while (hasMore)
    {
        if(state_==kExpectRequestLine){
            const char* crlf=buffer->findCRLF();
            if(crlf){
                ok=parseRequestLine(buffer->peek(),crlf);
                if(ok){
                    buffer->retrieve(2+crlf-buffer->peek());
                    state_=kExpectHeaders;
                }
                else{
                    hasMore=false;
                }
            }
            else{
                hasMore=false;
            }
        }
        else if(state_==kExpectHeaders){
            const char* crlf=buffer->findCRLF();
            if(crlf){
                if(crlf==buffer->peek()){
                    // 空行 
                    // LOG_INFO<<"HttpContext::parseRequest parseHeader finish\n";
                    if(httpRequest_.method()==HttpRequest::kPost){
                        state_=kExpectBody;
                    }
                    else {
                        state_=kGotAll;
                        hasMore=false;
                    }
                }
                else{
                    const char* colon = std::find(buffer->peek(), crlf, ':');
                    if(colon!=crlf){
                        //addHeader
                        httpRequest_.addHeader(buffer->peek(),colon,crlf);
                    }
                    else{
                        ok=false;
                        hasMore=false;
                    }
                }
                buffer->retrieve(2+crlf-buffer->peek());
            }
            else{
                hasMore=false;
            }
        }
        else if(state_==kExpectBody){
            ok=parseRequestBody(buffer);
            hasMore=false;
        }
        else{
            hasMore=false;
        }
    }
    return ok;
}