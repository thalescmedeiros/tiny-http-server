#ifndef H_HttpServerRoute
#define H_HttpServerRoute

#include <string>

enum REQUEST_TYPE {
    SERVER_COMMAND = 1,
    FILE_FROM_SYSTEM,
    FRIENDLY_PATH,
    WEB_SOCKET
};

class HttpServerRoute
{
    public:
        HttpServerRoute() { }
        HttpServerRoute(int idPage, std::string url, std::string method, bool mustBeAuthenticated, REQUEST_TYPE requestType)
            : idPage(idPage), url(url), method(method), mustBeAuthenticated(mustBeAuthenticated), requestType(requestType) { }
        HttpServerRoute(int idPage, std::string url, std::string pathMapped, std::string method, bool mustBeAuthenticated, REQUEST_TYPE requestType)
            : idPage(idPage), url(url), pathMapped(pathMapped), method(method), mustBeAuthenticated(mustBeAuthenticated), requestType(requestType) { }

        int GetIdPage() { return idPage; }
        std::string GetUrl() { return url; }
        std::string GetPathMapped() { return pathMapped; }
        std::string GetMethod() { return method; }
        bool GetMustBeAuthenticated() { return mustBeAuthenticated; }
        REQUEST_TYPE GetRequestType() { return requestType; }
        bool PathMatchesUrlPattern(std::string path);

    private:
        int idPage;
        bool mustBeAuthenticated;
        std::string url;
        std::string method;
        std::string pathMapped;
        REQUEST_TYPE requestType;
};

#endif
