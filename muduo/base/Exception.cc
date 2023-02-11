#include "muduo/base/Exception.h"

#include <cxxabi.h>

#include<execinfo.h>
namespace muduo
{

Exception::Exception(string what):message_(what),stack_(fillStackTrace(true)){}

string Exception::fillStackTrace(bool demangle){
    string stack;
    const int max_frames=200;
    void* buf[max_frames];
    int nptr=backtrace(buf,max_frames);
    char ** strings=backtrace_symbols(buf,nptr);
    if(strings){
        for (size_t i = 1; i < nptr; i++){
            /* code */
            size_t len = 256;
            char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
            if (demangle){
                // https://panthema.net/2008/0901-stacktrace-demangled/
                // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
                char* left_par = nullptr;
                char* plus = nullptr;
                for (char* p = strings[i]; *p; ++p)
                {
                if (*p == '(')
                    left_par = p;
                else if (*p == '+')
                    plus = p;
                }

                if (left_par && plus)
                {
                *plus = '\0';
                int status = 0;
                char* ret = abi::__cxa_demangle(left_par+1, demangled, &len, &status);
                *plus = '+';
                if (status == 0)
                {
                    demangled = ret;  // ret could be realloc()
                    stack.append(strings[i], left_par+1);
                    stack.append(demangled);
                    stack.append(plus);
                    stack.push_back('\n');
                    continue;
                }
                }
            }
            stack.append(strings[i]);
            stack.push_back('\n');
        }
    }
    free(strings);
    return stack;
}
} // namespace muduo
