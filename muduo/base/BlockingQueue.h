#ifndef _MUDUO_BASE_BLOCKINGQUEUE_H_
#define _MUDUO_BASE_BLOCKINGQUEUE_H_

#include<queue>

#include"muduo/base/Mutex.h"
#include"muduo/base/Condition.h"
#include"muduo/base/noncopyable.h"


namespace muduo
{

template<typename T>
class BlockingQueue: public noncopyable
{
public:
    typedef std::deque<T> queue_t;
private:
    /* data */
    queue_t queue_;
    mutable MutexLock mutex_;
    Condition notEmpty_;

public:
    BlockingQueue(/* args */):queue_(),mutex_(),notEmpty_(mutex_){}
    ~BlockingQueue(){}
    void put(T&& x){
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }
    void put(const T& x){
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify();
    }
    T take(){
        MutexLockGuard lock(mutex_);
        while (queue_.empty())
        {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T ret(std::move(queue_.front()));
        // T ret=queue_.front();
        // queue_.pop();
        queue_.pop_front();
        return ret;
    }
    queue_t drain(){
        queue_t queue;
        {
            MutexLockGuard lock(mutex_);
            queue=std::move(queue_);
            assert(queue_.empty());
        }
        return queue;
    }
    size_t size()const{
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
};




} // namespace muduo


#endif