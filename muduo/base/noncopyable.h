#ifndef _MUDUO_BASE_NONCOPYABLE_H_
#define _MUDUO_BASE_NONCOPYABLE_H_


namespace muduo{

class noncopyable
{
public:
    /* data */
    noncopyable(const noncopyable&)=delete;
    void operator= (const noncopyable&)=delete;
protected:
    noncopyable(/* args */)=default;
    ~noncopyable()=default;
};


}// namespace uncopyable


#endif