#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

#include<iostream>
#include<pthread.h>
#include<sys/time.h>
#include<unistd.h>

using namespace muduo;
using namespace std;

void timeSet(int year,int month,int day,int hour,int min,int sec)
{
    struct tm tptr;
    struct timeval tv;

    tptr.tm_year = year - 1900;
    tptr.tm_mon = month - 1;
    tptr.tm_mday = day;
    tptr.tm_hour = hour;
    tptr.tm_min = min;
    tptr.tm_sec = sec;

    tv.tv_sec = mktime(&tptr);
    tv.tv_usec = 0;
    system("date");
    if(settimeofday(&tv, NULL)!=0){
        if(errno==EPERM){
            printf("permisson denied.please use root\n");
            exit(0);
        }
    }
    system("date");
    return;
}

void* testSetTime(void* p){
    timeSet(2012,10,10,10,10,10);
    sleep(15);
    printf("settime thread sleep 15s and end\n");
    return NULL;
}

void test_timeout(){
    MutexLock mutex;
    Condition cond(mutex);
    pthread_t thread;
    pthread_create(&thread,NULL,testSetTime,NULL);
    mutex.lock();
    cond.waitForSeconds(10);
    mutex.unlock();
}

int main(){
    test_timeout();
}