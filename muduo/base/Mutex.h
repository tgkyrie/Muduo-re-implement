#ifndef _MUDUO_BASE_MUTEX_H_
#define _MUDUO_BASE_MUTEX_H_

#include<pthread.h>
#include<assert.h>
#include<muduo/base/CurrentThread.h>
#include<muduo/base/noncopyable.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace muduo
{
class MutexLock: public noncopyable
{
private:
    /* data */
    friend class Condition;
    pthread_mutex_t mutex_;
    pid_t holder_;

    void assignHolder(){
        holder_=t_currentThread.tid();
    }
    void unassignHolder(){
        holder_=0;
    }

    class UnassignGuard
    {
    private:
        /* data */
        MutexLock& mutex_;
    public:
        UnassignGuard(MutexLock& mutex):mutex_(mutex){
            mutex_.unassignHolder();
        }
        ~UnassignGuard(){
            mutex_.assignHolder();
        }
    };
    

public:
    MutexLock():holder_(0){
        pthread_mutex_init(&mutex_,NULL);
    }
    ~MutexLock(){
        assert(holder_==0);
        pthread_mutex_destroy(&mutex_);
    }
    void lock(){
        MCHECK(pthread_mutex_lock(&mutex_));
        assignHolder();
    }

    void unlock(){
        unassignHolder();
        pthread_mutex_unlock(&mutex_);
    }
    bool isLockedByThisThread() const
    {
        return holder_ == t_currentThread.tid();
    }

    void assertLocked() const
    {
        assert(holder_!=0);
        assert(isLockedByThisThread());
    }

    pthread_mutex_t* getPthreadMutex(){
        return &mutex_;
    }

};

class MutexLockGuard :public noncopyable
{
private:
    /* data */
    MutexLock& mutex_;
public:
    explicit MutexLockGuard(MutexLock& mutex):mutex_(mutex){
        mutex_.lock();
    }
    ~MutexLockGuard(){
        mutex_.unlock();
    }
};




} // namespace muduo

#define MutexLockGuard(x) error "Missing guard object name"
#define UnassignGuard(x) error "Missing guard object name"


#endif