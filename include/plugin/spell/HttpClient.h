#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

// HttpClient：通用 HTTP 客户端，封装 WinHTTP 传输层
// 职责单一：发送 HTTP POST 请求并返回响应体，不关心请求/响应的业务格式
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // 发送 POST 请求，返回响应体字符串
    std::string post(const std::string& url, const std::string& body,
                     const std::string& contentType = "application/x-www-form-urlencoded");
};

#endif // HTTPCLIENT_H
