#include "muduo/base/CountDownLatch.h"

namespace muduo{

CountDownLatch::CountDownLatch(int count):
    mutex_(),
    cond_(mutex_),
    count_(count)
{
}

CountDownLatch::~CountDownLatch()
{
}

int CountDownLatch::getCount() const{
    return count_;
}

void CountDownLatch::countDown(){
    MutexLockGuard lock(mutex_);
    count_--;
    if(count_==0){
        cond_.notifyAll();
    }
}

void CountDownLatch::wait(){
    MutexLockGuard lock(mutex_);
    while (count_>0)
    {
        cond_.wait();
    }
}

} // namespace muduo

