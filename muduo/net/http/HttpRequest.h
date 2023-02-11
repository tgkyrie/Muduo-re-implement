#if !defined(_MUDUO_NET_HTTP_HTTPREQUEST_H_)
#define _MUDUO_NET_HTTP_HTTPREQUEST_H_
#include"muduo/base/copyable.h"
#include"muduo/base/Log.h"
#include<map>
#include<string>
#include<algorithm>
#include<vector>
namespace muduo
{
namespace net
{
namespace http
{
class HttpRequest:copyable
{
public:
    enum Version{
        kHttp10,
        kHttp11,
        kUnkown
    };
    enum Method{
        kGet,
        kPost,
        kHead,
        kInvalid
    };
    const static std::vector<std::string> kMethodStrings;
    HttpRequest():method_(kInvalid),version_(kUnkown){}

    ~HttpRequest()=default;
    void setVersion(Version v){
        //LOG_TRACE<<"HttpRequest::setVersion()"<<v<<"\n";
        version_=v;
    }
    bool setMethod(const char* begin,const char* end){
        //LOG_TRACE<<"HttpRequest::setMethod()\n";
        std::string method(begin,end);
        int i=0;
        for(int i=0;i<kMethodStrings.size();i++){
            if(method==kMethodStrings[i]){
                method_=(Method)i;
                return true;
            }
        }
        method_=kInvalid;
        //LOG_TRACE<<"HttpRequest::setMethod() Unkown Method"<<method<<"\n";
        return false;
    }
    void setPath(const char* begin,const char* end){
        //LOG_TRACE<<"HttpRequest::setPath()\n";
        path_.assign(begin,end);
    }
    void setQuery(const char* begin,const char* end){
        //LOG_TRACE<<"HttpRequest::setQuery()\n";
        query_.assign(begin,end);
    }
    void addHeader(const char* begin,const char* colon,const char* end){
        std::string k(begin,colon);
        trim(k);
        std::transform(k.begin(),k.end(),k.begin(),tolower);
        std::string v(colon+1,end);
        trim(v);
        std::transform(v.begin(),v.end(),v.begin(),tolower);
        header_[k]=v;
    }
    void swap(HttpRequest& that){
        std::swap(method_,that.method_);
        std::swap(version_,that.version_);
        std::swap(path_,that.path_);
        std::swap(header_,that.header_);
        std::swap(query_,that.query_);
    }
    std::string getHeader(const std::string& k)const{
        std::string result="";
        std::map<std::string, std::string>::const_iterator it = header_.find(k);
        if (it != header_.end())
        {
            result = it->second;
        }
        return result;
    }
    Version getVersion()const{return version_;}
    std::string path()const {return path_;}
    std::string query()const {return query_;}
    Method method()const{return method_;}
    std::string methodString()const{
        return kMethodStrings[method_];
    }
    const std::map<std::string,std::string>& header()const{
        return header_;
    }
    void setBody(const std::string& body){body_=body;}
    void setBody(const char* begin,const char* end){
        body_.assign(begin,end);
    }
    const std::string& body()const{return body_;}
private:
    Method method_;
    Version version_;
    std::string path_;
    std::map<std::string,std::string> header_;
    std::string query_;
    std::string body_;
    inline void trim(std::string& s){
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
};



    
} // namespace http

    
} // namespace net

    
} // namespace muduo



#endif // _MUDUO_NET_HTTP_HTTPREQUEST_H_
