#ifndef _MUDUO_BASE_COUNTDOWNLATCH_H_
#define _MUDUO_BASE_COUNTDOWNLATCH_H_
#include"muduo/base/Condition.h"
#include"muduo/base/Mutex.h"

namespace muduo
{
    
class CountDownLatch
{
private:
    /* data */
    MutexLock mutex_;
    Condition cond_;
    int count_;
public:
    explicit CountDownLatch(int count);
    ~CountDownLatch();
    int getCount() const;
    void countDown();
    void wait();
};


} // namespace muduo


#endif