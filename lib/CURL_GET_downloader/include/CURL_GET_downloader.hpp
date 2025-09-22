#pragma once
#ifndef HPP__CURL_GET_downloader
#define HPP__CURL_GET_downloader


#include <string>
#include <curl/curl.h>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <functional>

namespace CGd {

    std::stringstream verboseStream;
    // using callback_progressFuncType = std::function<int(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)>;
    // using callback_progressFuncType = int(*)(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    // typedef int (callback_progressFuncType)(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    class HttpClient {
    public:

        HttpClient();
        ~HttpClient();

        bool downloadFile(const std::string& url, const std::string& outputPath);
        std::string getLastError() const;
        void setTimeout(long seconds);
        void setSslVerify(bool verify);

        int (*progressCallback_func)(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    private:
        CURL* curl;
        std::string errorBuffer;
        long timeout;
        bool sslVerify;
        
        static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    };
};

#endif //CURL_GET_downloader