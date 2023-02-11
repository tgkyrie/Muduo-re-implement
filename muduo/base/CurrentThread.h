#ifndef _MUDUO_BASE_CURRENTTHREAD_H_
#define _MUDUO_BASE_CURRENTTHREAD_H_

#include<pthread.h>
#include "muduo/base/noncopyable.h"
#include "muduo/base/Types.h"


namespace muduo
{

class CurrentThread :public noncopyable
{
private:
    /* data */
    int cacheTid_;
    char tidString_[32];
    int tidStringLength_;
    const char* threadName_;
public:
    CurrentThread():cacheTid_(0),tidStringLength_(6),threadName_("unkown"){
        return ;
    }
    ~CurrentThread(){}
    int tid();
    const char* tidString();
    int tidStringLength();
    bool isMainThread();
    const char* threadName();
    void cacheTid();
    void sleepUsec(int64_t usec);
    void setTidString(const char* tidString); //set tidstring and string length
    void setThreadName(const char* threadName);

};

extern thread_local CurrentThread t_currentThread;

} // namespace muduo


#endif