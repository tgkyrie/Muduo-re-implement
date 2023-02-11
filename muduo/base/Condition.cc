#include "muduo/base/Condition.h"

#include<pthread.h>
#include<errno.h>

namespace muduo
{
    
void Condition::wait(){
    // mutex_.unassignHolder();
    assert(mutex_.isLockedByThisThread());
    MutexLock::UnassignGuard unassignHolder(mutex_);
    pthread_cond_wait(&cond_,mutex_.getPthreadMutex());
    // mutex_.assignHolder();
}

void Condition::notify(){
    pthread_cond_signal(&cond_);
}

void Condition::notifyAll(){
    pthread_cond_broadcast(&cond_);
}

bool Condition::waitForSeconds(double seconds){
    struct timespec abstime;
    // FIX: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
    // clock_gettime(CLOCK_REALTIME, &abstime);
    clock_gettime(CLOCK_MONOTONIC,&abstime);
    const int64_t kNanoSecondsPerSecond = 1000000000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

    assert(mutex_.isLockedByThisThread());
    MutexLock::UnassignGuard unassignHolder(mutex_);
    // mutex_.unassignHolder();
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getPthreadMutex(), &abstime);
}

} // namespace muduo
