#ifndef ARDUINO

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "GlobalSettings.h"
#include "TimerControl.h"
#include "HttpServerControl.h"
#include "HttpClientBase.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpResponseFile.h"
#include "HttpBuilder.h"

bool HttpResponseFile::FileExists(std::string localFolder, std::string urlPath)
{
    std::string filename = localFolder + urlPath;
    struct stat st;
    return stat(filename.c_str(), &st) == 0;
}

void HttpResponseFile::InitializeResponse(std::unique_ptr<HttpClientBase> &client)
{
    //-----------------------------------------------------------------
    // HttpServerRoute::PathMatchesUrlPattern() was called before:
    // => So the PATH is trustable
    // HttpResponseFile::FileExists() was called before:
    // => So the files exists!!!
    //-----------------------------------------------------------------
    HttpServerRoute *route = client->GetCurrentRequest()->GetRoute();
    std::string localFolder = client->GetServer()->GetApp()->GetLocalFolder();
    std::string requestedPath = route->GetRequestType() == FRIENDLY_PATH ? route->GetPathMapped() : client->GetCurrentRequest()->GetPath();
// std::cout << "requestedPath: " << requestedPath << "\n";

    this->fullPath = localFolder + requestedPath;
// std::cout << "path: " << this->fullPath << "\n";
    
    // Getting file size
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = file.tellg();
    file.close();
    this->bytesSent = 0;
// std::cout << "size: " << std::to_string(fileSize) << "\n";
    std::cout << "requestedPath: " << requestedPath << " (" << std::to_string(fileSize) << " bytes)\n";

    // Getting extension
    std::string extension = "";
    std::string::size_type idx = fullPath.rfind('.');
    if(idx != std::string::npos) extension = fullPath.substr(idx+1);
// std::cout << "extension: " << extension << "\n";

    // Getting mime/type
    std::string mime = HttpBuilder::GetMimeTypeByExtension(extension);
// std::cout << "mime: " << mime << "\n";

    // Initializing the response
    this->SetStatusCode(200);
	if (mime.size() > 0) this->AddHeader("Content-Type", mime);
	this->SetContentLength(fileSize);
}

bool HttpResponseFile::WriteFileToClient(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    int remainingBytes = this->GetContentLength() - this->bytesSent;

    std::ifstream file(fullPath, std::ios::binary);
    file.seekg(bytesSent);

    // std::cout << "remainingBytes: " << remainingBytes << "\n";
    while (remainingBytes > 0)
    {
        int bytesToWrite = remainingBytes > HTTP_WRITE_BUFFER_SIZE ? HTTP_WRITE_BUFFER_SIZE : remainingBytes;

        std::vector<char> buffer(bytesToWrite);
        if (file.read(buffer.data(), bytesToWrite))
        {
            client->WriteData(buffer);
        }    

        bytesSent += bytesToWrite;
        remainingBytes -= bytesToWrite;

        // Checking if timeout
        double timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(GetCustomCurrentTime() - timeStart);
        if (timeEllapsed >= MAX_PROCESSING_TIME && remainingBytes > 0) break;
    }
    file.close();

    bool result = remainingBytes == 0;
    return result;
}

#endif
