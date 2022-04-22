#ifndef H_HttpResponse
#define H_HttpResponse

#include <memory>
#include <vector>
#include <string>

class HttpResponse
{
    public:
        HttpResponse() : headers(""), headersSent(false), contentLength(0) { }
        HttpResponse(int StatusCode, std::vector<char> &Content)
            : headers(""), statusCode(StatusCode), content(Content), headersSent(false), contentLength(0)
        { }
        
        void AddHeader(std::string Header, std::string Value) { headers += Header + ": " + Value + "\r\n"; }
        std::string GetHeader(std::string header);
        bool ContainsHeader(std::string Header);
        int GetStatusCode() { return this->statusCode; }
        std::string GetReasonPhrase();
        std::string GetHeaders() { return this->headers; }
        std::string GetStringContent() { return std::string(content.begin(), content.end()); }
        std::vector<char> GetContent() { return this->content; }
        size_t GetContentLength() { return this->contentLength; }
        bool GetHeadersSent() { return this->headersSent; }
        void SetContentType(std::string Value);
        void SetContentText(std::string Value);
        void SetContentLength(size_t Value) { contentLength = Value; }
        void SetStatusCode(int statusCode) { this->statusCode = statusCode; }
        void SetContent(std::vector<char> content) { this->content = content; }
        void SetHeadersSent(bool value) { this->headersSent = value; }

    private:
        int statusCode;
        std::string headers;
        size_t contentLength;
        bool headersSent;
        std::vector<char> content;
};

#endif
