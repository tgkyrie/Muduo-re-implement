#include"muduo/net/http/HttpRequest.h"
using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::http;

const std::vector<std::string> HttpRequest::kMethodStrings={"GET","POST","HEAD","INVALID"};
