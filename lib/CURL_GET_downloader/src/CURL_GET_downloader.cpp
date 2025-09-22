
#include "CURL_GET_downloader.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

// Callback for writing received data to file
size_t CGd::HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    if (file && file->is_open()) {
        file->write(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
    return 0;
}

// Progress callback
int CGd::HttpClient::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (dltotal > 0) {
        double progress = static_cast<double>(dlnow) / static_cast<double>(dltotal) * 100.0;
        verboseStream << "\rDownload progress: " << progress << "% (" << dlnow << "/" << dltotal << " bytes)";
    }
    return 0;
}

CGd::HttpClient::HttpClient() : curl(nullptr), timeout(30), sslVerify(true) {
    progressCallback_func = progressCallback;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    errorBuffer.resize(CURL_ERROR_SIZE);
}

CGd::HttpClient::~HttpClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void CGd::HttpClient::setTimeout(long seconds) {
    timeout = seconds;
}

void CGd::HttpClient::setSslVerify(bool verify) {
    sslVerify = verify;
}

bool CGd::HttpClient::downloadFile(const std::string& url, const std::string& outputPath) {
    if (!curl) {
        errorBuffer = "CURL not initialized";
        return false;
    }

    // Reset curl session
    curl_easy_reset(curl);

    // Create output directory if it doesn't exist
    fs::path path(outputPath);
    fs::path parentDir = path.parent_path();
    
    // Only create directories if there's a parent path (not just filename)
    if (!parentDir.empty() && !fs::exists(parentDir)) {
        std::error_code ec;
        if (!fs::create_directories(parentDir, ec)) {
            errorBuffer = "Failed to create directory: " + parentDir.string() + " - Error: " + ec.message();
            return false;
        }
        verboseStream << "Created directory: " << parentDir.string() << "\n";
    }

    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile.is_open()) {
        errorBuffer = "Failed to open output file: " + outputPath;
        return false;
    }

    // Set required headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
    headers = curl_slist_append(headers, "Accept: application/octet-stream");
    headers = curl_slist_append(headers, "User-Agent: ProtobufDownloader/1.0");
    headers = curl_slist_append(headers, "Connection: keep-alive");

    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer.data());
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, sslVerify ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, sslVerify ? 2L : 0L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    if(progressCallback_func) curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback_func);
    
    // Enable automatic decompression
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    verboseStream << "Starting download from: " << url << "\n";
    verboseStream << "Saving to: " << fs::absolute(path).string() << "\n";
    verboseStream << "Headers set: Accept-Encoding: gzip, deflate" << "\n";

    CURLcode res = curl_easy_perform(curl);
    
    // Clean up headers
    curl_slist_free_all(headers);
    outputFile.close();

    if (res != CURLE_OK) {
        // Clean up partially downloaded file
        if (fs::exists(outputPath)) {
            fs::remove(outputPath);
        }
        return false;
    }

    verboseStream << "\nDownload completed successfully!" << "\n";
    return true;
}

std::string CGd::HttpClient::getLastError() const {
    return errorBuffer;
}

