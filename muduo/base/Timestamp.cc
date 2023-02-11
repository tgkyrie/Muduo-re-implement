#include "muduo/base/Timestamp.h"

#include<stdio.h>
#include<sys/time.h>

namespace muduo{
    
string Timestamp::toString() const{
    int64_t seconds=microSecondsSinceEpoch_/kMicroSecondsPerSecond;
    int64_t microseconds=microSecondsSinceEpoch_%kMicroSecondsPerSecond;
    char buf[32]={0};
    snprintf(buf,sizeof(buf),"%lld.%06lld",seconds,microseconds);
    return buf;
}

string Timestamp::toFormattedString(bool showMicroSeconds) const{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if(showMicroSeconds){
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds);
    }
    else{
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

} //namespace muduo