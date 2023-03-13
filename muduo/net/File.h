#ifndef _MUDUO_NET_FILE_H_
#define _MUDUO_NET_FILE_H_
#include<string>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include"muduo/net/Callbacks.h"
namespace muduo
{
namespace net
{
class File
{
public:
    File(const std::string fileName){
        fd_=::open(fileName.c_str(),O_CLOEXEC|O_RDONLY);
        if(fd_>=0){
            struct stat buf;
            fstat(fd_,&buf);
            fileSize_=buf.st_size; 
        }
        else fileSize_=0;
    } 
    ~File(){
        if(fd_>=0)::close(fd_);
    }
    int fd()const {return fd_;}
    size_t fileSize()const {return fileSize_;}
private:
    int fd_;
    size_t fileSize_;
};
} // namespace net

} // namespace muduo




#endif