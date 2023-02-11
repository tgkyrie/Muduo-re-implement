#ifndef _MUDUO_BASE_EXCEPTION_H_
#define _MUDUO_BASE_EXCEPTION_H_


#include "muduo/base/Types.h"
namespace muduo
{

class Exception :public std::exception
{
private:
    /* data */
    string message_;
    string stack_;
public:
    Exception(string what);
    ~Exception(){}
    string fillStackTrace(bool demangle=false);
    const char* what() const noexcept{
        return message_.c_str();
    }
    const char* stackTrace() const noexcept{
        return stack_.c_str();
    }
};



} // namespace muduo


#endif