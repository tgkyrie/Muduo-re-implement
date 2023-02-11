#ifndef _MUDUO_BASE_ATOMIC_H_
#define _MUDUO_BASE_ATOMIC_H_

#include"muduo/base/noncopyable.h"
#include<stdint.h>

//__sync_val_compare_and_swap(ptr,oldVal,newVal)
//__sync_fetch_and_add(ptr,val)
//__sync_lock_test_and_set(ptr ,val)

namespace muduo
{

template<typename T>
class AtomicIntegerT:public noncopyable
{
private:
    /* data */
    T value_;
public:
    AtomicIntegerT():value_(0){}
    // AtomicIntegerT(T value):value_(value){}
    ~AtomicIntegerT(){}
    T get(){
        return __sync_val_compare_and_swap(&value_,0,0);
    }
    T getAndAdd(T x){
        return __sync_fetch_and_add(&value_,x);
    }
    T addAndGet(T x){
        return getAndAdd(x)+x;
    }
    T incrementAndGet(){
        return getAndAdd(1)+1;
    }
    T decrementAndGet(){
        return getAndAdd(-1)-1;
    }
    void add(T x){
        getAndAdd(x);
    }
    void increment(){
        add(1);
    }
    void decrement(){
        add(-1);
    }
    T getAndSet(T newValue){
        return __sync_lock_test_and_set(&value_,newValue);
    }

};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;


} // namespace muduo
#endif