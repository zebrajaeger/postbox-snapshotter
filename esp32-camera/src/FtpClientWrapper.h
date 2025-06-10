
#pragma once

#include <Arduino.h>
#include <WiFiClient.h> 
#include <ESP32_FTPClient.h>
#include <functional>

class FtpClientWrapper {
    public:
        using DownloadCallback = std::function<void(Stream& stream, size_t size)>;
    
        FtpClientWrapper(const char* server, const char* user, const char* pass);
        bool connect();
        void disconnect();
        bool changeDir(const char* dir);
        bool upload(const uint8_t* buf, size_t len, const char* filename);
        size_t fileSize(const char* filename);
        bool download(const char* filename, DownloadCallback callback);
        bool renameFile(const char* oldName, const char* newName);
    
    private:
        ESP32_FTPClient _ftp;
        bool _connected;
    };