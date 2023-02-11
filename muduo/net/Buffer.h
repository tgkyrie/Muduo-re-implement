#if !defined(_MUDUO_NET_BUFFER_H_)
#define _MUDUO_NET_BUFFER_H_

#include<vector>
#include<string.h>
#include<unistd.h>
#include<string>
#include<assert.h>
#include<algorithm>
#include"muduo/base/Log.h"
namespace muduo
{
namespace net
{

class Buffer
{

public:
    static const size_t kCheapPrepend=8;
    static const size_t kInitSize=1024;
    Buffer(size_t initSize=kInitSize):readIndex_(kCheapPrepend),
        writeIndex_(kCheapPrepend),
        buffer_(initSize){}
    ~Buffer()=default;
    size_t readableBytes()const{
        return writeIndex_-readIndex_;
    }
    size_t writableBytes()const{
        return buffer_.size()-writeIndex_;
    }
    size_t prependableBytes(){
        return readIndex_;
    }
    char* begin(){
        return &buffer_[0];
    }
    const char* begin()const{
        return &buffer_[0];
    }
    const char* peek()const{
        return begin()+readIndex_;
    }
    const char* beginWrite()const{
        return begin()+writeIndex_;
    }
    ssize_t readFd(int fd,int& savedErrno);
    void append(const std::string& s);
    void append(const char* data){append(data,strlen(data));}
    void append(const char* data,size_t len);
    void retrieve(size_t len);
    void retrieveAll();
    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();

    const char* findCRLF() const
    {
        //LOG_INFO<<"Buffer::findCRLF()\n";
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        //LOG_INFO<<"Buffer::findCRLF()\n";
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }
private:
    void ensureWritableBytes(size_t len);
    void makeSpace(size_t len);
    /* data */

    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
    static const char kCRLF[];
};



} // namespace net
    
} // namespace muduo




#endif // _MUDUO_NET_BUFFER_H_
