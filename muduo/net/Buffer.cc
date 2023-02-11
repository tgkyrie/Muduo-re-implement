#include"muduo/net/Buffer.h"

#include<sys/uio.h>
#include<errno.h>
using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[]="\r\n";
ssize_t Buffer::readFd(int fd,int& savedErrno){
    char buf[65536];
    struct iovec vec[2];
    const size_t writable=writableBytes();
    vec[0].iov_len=writableBytes();
    vec[0].iov_base=begin()+writeIndex_;
    vec[1].iov_len=sizeof buf;
    vec[1].iov_base=&buf;

    int iocnt=writableBytes()<sizeof buf?2:1;
    ssize_t n=readv(fd,vec,iocnt);
    if(n<0){
        savedErrno=errno;
    }
    else if(static_cast<size_t>(n)<writableBytes()){
        writeIndex_+=n;
    }
    else{
        writeIndex_=buffer_.size();
        append(buf,n-writable);
    }
    return n;

}


void Buffer::retrieve(size_t len){
    if(readableBytes()>=len){
        readIndex_+=len;
    }
    else{
        readIndex_+=readableBytes();
    }
}

void Buffer::retrieveAll(){
    readIndex_+=readableBytes();
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}
void Buffer::append(const std::string& s){
    append(s.c_str(),s.size());
}

void Buffer::append(const char* data,size_t len){
    ensureWritableBytes(len);
    std::copy(data,data+len,begin()+writeIndex_);
    writeIndex_+=len;
}

void Buffer::ensureWritableBytes(size_t len){
    if(writableBytes()<len){
        makeSpace(len);
    }
}
void Buffer::makeSpace(size_t len){
    if(writableBytes()+prependableBytes()-kCheapPrepend<len){
        buffer_.resize(writeIndex_+len);
    }
    else{
        size_t readable=readableBytes();
        std::copy(begin()+readIndex_,begin()+writeIndex_,begin()+kCheapPrepend);
        readIndex_=kCheapPrepend;
        writeIndex_=readIndex_+readable;
    }
}