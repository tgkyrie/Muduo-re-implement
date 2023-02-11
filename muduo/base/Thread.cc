#include "muduo/base/Thread.h"
#include "muduo/base/CurrentThread.h"
#include"muduo/base/Exception.h"

#include<sys/prctl.h>

namespace muduo
{

namespace detail
{
    
struct ThreadData
{
    /* data */
    typedef muduo::Thread::ThreadFunc_t ThreadFunc_t;
    ThreadFunc_t func_;
    pid_t* tid_;
    string name_;
    CountDownLatch* latch_;
    ThreadData(ThreadFunc_t func,pid_t* tid,string name,CountDownLatch* latch):
        func_(std::move(func)),
        tid_(tid),
        name_(std::move(name)),
        latch_(latch){}
    
    void runInThread(){
        *tid_=t_currentThread.tid();
        latch_->countDown();
        tid_=NULL;
        t_currentThread.setThreadName(name_.c_str());
        prctl(PR_SET_NAME,t_currentThread.threadName());
        try
        {
            func_();
            t_currentThread.setThreadName("finished");
        }
        catch(const Exception& ex)
        {
            // std::cerr << e.what() << '\n';
            t_currentThread.setThreadName("crashed");
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        }
        catch(const std::exception& ex){
            t_currentThread.setThreadName("crashed");
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
        }
        catch(...){
            t_currentThread.setThreadName("crashed");
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
        
    }
    
};

void* startThread(void* aux){
    ThreadData * data=static_cast<ThreadData*>(aux);
    data->runInThread();
    return NULL;
}

} // namespace detail




AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc_t func,string name):
    started_(false),
    joined_(false),
    name_(std::move(name)),
    func_(func),
    tid_(0),
    pthreadId_(0),
    latch_(1)
{
    setDefaultName();
}

void Thread::start(){
    assert(!started_);
    started_=true;
    detail::ThreadData* data=new detail::ThreadData(func_,&tid_,name_,&latch_);
    if(pthread_create(&pthreadId_,NULL,detail::startThread,data)){
        started_=false;
        delete data;
        // LOG_SYSFATAL << "Failed in pthread_create"; //LOG
    }
    else {
        latch_.wait();
        assert(tid_>0);
    }
}
int Thread::join(){
    assert(started_&&!joined_);
    joined_=true;
    return pthread_join(pthreadId_,NULL);
}

void Thread::setDefaultName(){
    int num=numCreated_.incrementAndGet();
    if(name_.empty()){
        char buf[32];
        snprintf(buf,32,"Thread%d",num);
        name_=buf;
    }
}

} // namespace muduo
