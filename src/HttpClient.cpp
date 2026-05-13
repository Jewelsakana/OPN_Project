#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <stdexcept>
#include <vector>

namespace {

std::wstring toWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

bool parseUrl(const std::string& url, std::wstring& host, WORD& port, std::wstring& path) {
    std::string remaining = url;
    if (remaining.substr(0, 8) == "https://") {
        remaining = remaining.substr(8);
        port = INTERNET_DEFAULT_HTTPS_PORT;
    } else if (remaining.substr(0, 7) == "http://") {
        remaining = remaining.substr(7);
        port = INTERNET_DEFAULT_HTTP_PORT;
    } else {
        return false;
    }

    size_t slashPos = remaining.find('/');
    if (slashPos != std::string::npos) {
        host = toWide(remaining.substr(0, slashPos));
        path = toWide(remaining.substr(slashPos));
    } else {
        host = toWide(remaining);
        path = L"/";
    }
    return true;
}

std::string readResponseBody(HINTERNET hRequest) {
    std::string body;
    DWORD bytesAvailable = 0;
    do {
        bytesAvailable = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) break;
        if (bytesAvailable == 0) break;

        std::vector<char> buffer(bytesAvailable + 1, '\0');
        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
            body.append(buffer.data(), bytesRead);
        } else {
            break;
        }
    } while (bytesAvailable > 0);
    return body;
}

} // anonymous namespace

HttpClient::HttpClient() {}
HttpClient::~HttpClient() {}

std::string HttpClient::post(const std::string& url, const std::string& body,
                              const std::string& contentType) {
    std::wstring host;
    WORD port = 0;
    std::wstring path;

    if (!parseUrl(url, host, port, path)) {
        throw std::runtime_error("HttpClient: Failed to parse URL: " + url);
    }

    HINTERNET hSession = WinHttpOpen(L"TextEditor/1.0",
                                      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        throw std::runtime_error("HttpClient: WinHttpOpen failed");
    }

    WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 10000);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("HttpClient: WinHttpConnect failed");
    }

    DWORD flags = (port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                             nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("HttpClient: WinHttpOpenRequest failed");
    }

    std::wstring headers = toWide("Content-Type: " + contentType + "\r\n");
    std::wstring bodyWide = toWide(body);

    BOOL result = WinHttpSendRequest(hRequest, headers.c_str(),
                                      static_cast<DWORD>(headers.size()),
                                      const_cast<wchar_t*>(bodyWide.c_str()),
                                      static_cast<DWORD>(bodyWide.size() * sizeof(wchar_t)),
                                      static_cast<DWORD>(bodyWide.size() * sizeof(wchar_t)), 0);
    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("HttpClient: WinHttpSendRequest failed");
    }

    result = WinHttpReceiveResponse(hRequest, nullptr);
    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("HttpClient: WinHttpReceiveResponse failed");
    }

    std::string responseBody = readResponseBody(hRequest);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (responseBody.empty()) {
        throw std::runtime_error("HttpClient: Empty response from server");
    }

    return responseBody;
}
