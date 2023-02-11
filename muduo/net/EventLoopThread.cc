#include"muduo/net/EventLoopThread.h"
using namespace muduo;
using namespace muduo::net;



EventLoopThread::EventLoopThread(const std::string& name):
name_(name),
loop_(nullptr),
latch_(1),
thread_(std::bind(&EventLoopThread::loop,this),name)
{
}

EventLoopThread::~EventLoopThread(){
    latch_.wait();
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop(){
    //run in old thread
    thread_.start();
    latch_.wait();
    return loop_;
}

void EventLoopThread::loop(){
    //run in new thread
    EventLoop loop;
    loop_=&loop;
    latch_.countDown();
    loop.loop();
}

