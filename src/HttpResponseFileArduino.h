#ifdef ARDUINO

#ifndef H_HttpResponseFileArduino
#define H_HttpResponseFileArduino

#include <SD.h>
#include <memory>
#include <string>
#include <fstream>
#include "TimerControl.h"
#include "HttpResponse.h"

class HttpClientBase;

class HttpResponseFile : public HttpResponse
{
    public:
        HttpResponseFile();

        bool FileExists(std::string localFolder, std::string urlPath);
        void InitializeResponse(std::unique_ptr<HttpClientBase> &client);
        bool WriteFileToClient(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart);

        static std::vector<char> ReadFileContent(std::string fullPath);

    private:
        std::string fullPath;
        std::streamsize bytesSent;
};

#endif

#endif
