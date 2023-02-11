#include "muduo/net/EventLoop.h"
// #include "muduo/net/EventLoopThread.h"
#include "muduo/base/Thread.h"
#include"muduo/net/TimerQueue.h"
#include<random>
#include <stdio.h>
#include <unistd.h>
#include<iostream>
#include"muduo/base/MinHeap.h"

using namespace muduo;
using namespace muduo::net;

int cnt = 0;
EventLoop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), t_currentThread.tid());
  printf("now %s\n", Timestamp::now().toString().c_str());
}

void print(const char* msg)
{
  printf("msg %s %s\n", Timestamp::now().toString().c_str(), msg);
  if (++cnt == 20)
  {
    g_loop->quit();
  }
}

void cancel(TimerId timer)
{
  g_loop->cancel(timer);
  printf("cancelled at %s\n", Timestamp::now().toString().c_str());
}
int gval=0;

void incgval(){
  // std::cout<<++gval<<std::endl;
  gval++;
}
void printgval()
{
  std::cout<<gval<<std::endl;
}
int main()
{
  printTid();
  sleep(1);
  // {
  //   int n=0;
  //   std::cin>>n;
  //   EventLoop loop;
  //   g_loop = &loop;
  //   srand(0);
  //   print("main");
  //   for(int i=0;i<n;i++){
  //     // double interval=10*(rand()%100)/100.0;
  //     // double canceltime=10*(rand()%100)/100.0;
  //     // int64_t t=loop.runEvery(interval,std::bind(print,"once"));
  //     // loop.runAfter(canceltime,std::bind(cancel,t));
  //     TimerId t=loop.runEvery(1.0,incgval);
  //     loop.runAfter(10.5,std::bind(cancel,t));
  //   }
  //   loop.runAfter(11.5,printgval);
  //   loop.runAfter(20.5,printgval);

  //   loop.loop();
  //   print("main loop exits");
  // }
 
  
  {
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(1.5, std::bind(print, "once1.5"));
    loop.runAfter(2.5, std::bind(print, "once2.5"));
    loop.runAfter(3.5, std::bind(print, "once3.5"));
    TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
    loop.runAfter(4.2, std::bind(cancel, t45));
    loop.runAfter(4.8, std::bind(cancel, t45));
    TimerId t2=loop.runEvery(2, std::bind(print, "every2"));
    TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
    // TimerQueue::Entry e1(Timestamp::now(),nullptr);
    // TimerQueue::Entry e2(addTime(Timestamp::now(),1),nullptr);
    // std::cout<<"e1:"<<e1.entry_.first.toString()<<std::endl;
    // std::cout<<"e2:"<<e2.entry_.first.toString()<<std::endl;
    // assert(e1<e2);
    // assert(e2<e1);
    
    loop.runAfter(9.001, std::bind(cancel, t3));

    loop.loop();
    print("main loop exits");
  }

//   sleep(1);
//   {
//     EventLoopThread loopThread;
//     EventLoop* loop = loopThread.startLoop();
//     loop->runAfter(2, printTid);
//     sleep(3);
//     print("thread loop exits");
//   }
}
