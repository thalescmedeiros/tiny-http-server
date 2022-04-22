#ifndef H_HttpRequest
#define H_HttpRequest

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include "HttpServerRoute.h"

class HttpRequest
{
    public:
        HttpRequest() : route(NULL) { }

        std::string GetMethod() { return this->method; }
        std::string GetPath() { return this->path; }
        std::string GetHeaders() { return this->headers; }
        std::string GetStringContent() { return std::string(content.begin(), content.end()); }
        std::vector<unsigned char> &GetContent() { return this->content; }
        size_t GetContentLength() { return content.size(); }
        void SetMethod(std::string &s) { this->method = s; }
        void SetPath(std::string &s) { this->path = s; }
        void SetHeaders(std::string &s) { this->headers = s; }
        void SetRoute(HttpServerRoute *route) { this->route = route; }
        HttpServerRoute *GetRoute() { return route; }

        std::string GetHeader(std::string &header);
        bool CheckHeaderValue(std::string &header, std::string &value);
        bool CheckHeaderContainsString(std::string &header, std::string &value);
        bool CheckHeaderContainsValueBetweenCommas(std::string &header, std::string &value);

    private:
        std::string method;
        std::string path;
        std::string headers;
        std::vector<unsigned char> content;
        HttpServerRoute *route;
};

#endif
