#ifndef _MUDUO_BASE_CONDITION_H_
#define _MUDUO_BASE_CONDITION_H_
#include "muduo/base/Mutex.h"

namespace muduo
{

class Condition: public noncopyable
{
private:
    /* data */
    MutexLock& mutex_;
    pthread_cond_t cond_;
    
public:
    explicit Condition(MutexLock& mutex):mutex_(mutex){
        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_condattr_setclock(&cond_attr,CLOCK_MONOTONIC);
        pthread_cond_init(&cond_,&cond_attr);
    }
    ~Condition(){}
    void wait();
    void notify();
    void notifyAll();
    bool waitForSeconds(double seconds);
};




} // namespace muduo



#endif  