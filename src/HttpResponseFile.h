#ifndef ARDUINO

#ifndef H_HttpResponseFile
#define H_HttpResponseFile

#include <memory>
#include <string>
#include <fstream>
#include "TimerControl.h"
#include "HttpResponse.h"

class HttpClientBase;

class HttpResponseFile : public HttpResponse
{
    public:
        HttpResponseFile() { }

        bool FileExists(std::string localFolder, std::string urlPath);
        void InitializeResponse(std::unique_ptr<HttpClientBase> &client);
        bool WriteFileToClient(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart);

    private:
        std::string fullPath;
        std::streamsize bytesSent;
};

#endif

#endif
