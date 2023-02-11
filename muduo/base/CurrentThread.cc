#include "muduo/base/CurrentThread.h"
#include"muduo/base/Types.h"
#include<unistd.h>
#include<sys/syscall.h>

// using muduo::CurrentThread;
// using namespace muduo;

namespace muduo
{

int CurrentThread::tid(){
    if (__builtin_expect(cacheTid_==0,0))
    {
        /* code */
        cacheTid();
    }
    return cacheTid_;
}

const char* CurrentThread::tidString(){
    return tidString_;
}

int CurrentThread::tidStringLength(){
    return tidStringLength_;
}

bool CurrentThread::isMainThread(){
    return tid()==getpid();
}

const char* CurrentThread::threadName(){
    return threadName_;
}

void CurrentThread::cacheTid(){
    if(cacheTid_==0){
        cacheTid_=static_cast<pid_t>(syscall(SYS_gettid));
        tidStringLength_=snprintf(tidString_,sizeof tidString_,"%5d",cacheTid_);
    }
}

void CurrentThread::setThreadName(const char* threadName){
    threadName_=threadName;
}


thread_local CurrentThread t_currentThread;



} ;// namespace muduo


// int tid();
// const char* tidString();
// int tidStringLength();
// bool isMainThread();
// const char* threadName();
