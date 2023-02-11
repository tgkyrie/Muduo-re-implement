#ifndef _MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H_
#define _MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H_

#include"muduo/base/Mutex.h"
#include"muduo/base/Condition.h"
#include"muduo/base/noncopyable.h"
#include<boost/circular_buffer.hpp>

namespace muduo
{
template<typename T>
class BoundedBlockingQueue :public noncopyable
{
private:
    /* data */
    mutable MutexLock mutex_;
    Condition notFull_;
    Condition notEmpty_;
    boost::circular_buffer<T> queue_;
public:
    explicit BoundedBlockingQueue(int maxSize):mutex_(),notFull_(mutex_),notEmpty_(mutex_),queue_(maxSize){}
    ~BoundedBlockingQueue(){}
    
    void put(const T& x){
        MutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(x);
        notEmpty_.notify();
    }
    
    void put(T&& x){
        MutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }

    T take(){
        MutexLockGuard lock(mutex_);
        while(queue_.empty()){
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notify();
        return front;
    }

    bool empty() const{
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    bool full() const{
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }

    size_t size() const{
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    size_t capacity() const{
        MutexLockGuard lock(mutex_);
        return queue_.capacity();
    }
};






} // namespace muduo


#endif