#ifndef _MUDUO_BASE_THREAD_H_
#define _MUDUO_BASE_THREAD_H_

#include"muduo/base/CountDownLatch.h"
#include"muduo/base/Atomic.h"
#include"muduo/base/Types.h"

#include<functional>


namespace muduo
{
    
class Thread
{


public:
    typedef std::function<void (void)> ThreadFunc_t;
    Thread(ThreadFunc_t func,string name=string());
    ~Thread(){}
    void start();
    int join(); //return pthread_join();
    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    string name() const {return name_;}
    static int numCreated() {return numCreated_.get();}

private:
    /* data */
    bool started_;
    bool joined_;
    pid_t tid_;
    pthread_t pthreadId_;
    CountDownLatch latch_;
    ThreadFunc_t func_;
    string name_;

    static AtomicInt32 numCreated_;
    
    void setDefaultName();
};



} // namespace muduo


#endif