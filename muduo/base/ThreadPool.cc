#include"muduo/base/ThreadPool.h"

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg):
    mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}
ThreadPool::~ThreadPool(){
    if(running_){
        stop();
    }    
}

void ThreadPool::start(int numThreads){
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for(int i=0;i<numThreads;i++){
        char id[32];
        snprintf(id,sizeof id,"%d",i+1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread,this),name_+id));
        threads_[i]->start();
    }
}

void ThreadPool::stop(){
    running_=false;
    {
        MutexLockGuard lock(mutex_);
        notEmpty_.notifyAll();
        notFull_.notifyAll();
    }
    for(auto& th:threads_){
        th->join();
    }
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task f){
    if(threads_.empty()){
        f();
    }
    else{
        MutexLockGuard lock(mutex_);
        while (isFull()&&running_)
        {
            notFull_.wait();
        }
        // mutex_.assertLocked();// 检测是否lock
        if (!running_) return;
        assert(!isFull());
        queue_.push_back(f);
        notEmpty_.notify();
    }
}

bool ThreadPool::isFull() const{
    mutex_.assertLocked();
    return queue_.size()>=maxQueueSize_&&maxQueueSize_>0;
}

void ThreadPool::runInThread(){
    Task task;
    while(running_){
        if(task=take())task();
    }
}

ThreadPool::Task ThreadPool::take(){
    MutexLockGuard lock(mutex_);
    while(queue_.empty()&&running_){
        notEmpty_.wait();
    }
    Task task;
    // mutex_.assertLocked();
    if(!queue_.empty()){
        task=queue_.front();
        queue_.pop_front();
    }
    return task;
}