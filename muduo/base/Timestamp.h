#ifndef _MUDUO_BASE_TIMESTAMP_H
#define _MUDUO_BASE_TIMESTAMP_H
#include "muduo/base/copyable.h"
#include "muduo/base/Types.h"
#include<boost/operators.hpp>

namespace muduo{

class Timestamp:public muduo::copyable,
                public boost::less_than_comparable<Timestamp>,
                public boost::equality_comparable<Timestamp>
{
private:
    /* data */
    int64_t microSecondsSinceEpoch_;
public:
    Timestamp(/* args */):microSecondsSinceEpoch_(0){}
    explicit Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){}
    ~Timestamp(){return ;};
    void swap(Timestamp& that){
        std::swap(microSecondsSinceEpoch_,that.microSecondsSinceEpoch_);
    }
    string toString() const;
    string toFormattedString(bool showMicroSeconds=true) const;
    bool valid() const { return microSecondsSinceEpoch_ > 0;}
    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }

    /// 从Unix时间戳构造Timestamp
    static Timestamp fromUnixTime(time_t t){
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microseconds){
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

    int64_t microSecondsSinceEpoch() const{return microSecondsSinceEpoch_;}
    time_t secondsSinceEpoch() const{ return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

    static const int kMicroSecondsPerSecond=1000*1000;


};  //class Timestamp

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
inline bool operator>(Timestamp lhs,Timestamp rhs){
  return lhs.microSecondsSinceEpoch()>rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}


}// namespace muduo

#endif