#ifndef H_HttpBuilder
#define H_HttpBuilder

class HttpClientBase;

class HttpBuilder
{
    public:
        static void FromJson(std::string &json, std::unique_ptr<HttpClientBase> &client);
        static void FromHtml(int StatusCode, std::string &html, std::unique_ptr<HttpClientBase> &client);
        static void FromHtml(std::string &html, std::unique_ptr<HttpClientBase> &client);

        static void WebSocketStartResponse(std::unique_ptr<HttpClientBase> &client);
        static void NotFound(std::unique_ptr<HttpClientBase> &client);
        static void BadRequest(std::unique_ptr<HttpClientBase> &client);
        static void InternalServerError(std::unique_ptr<HttpClientBase> &client);
        static void ServerTooBusy(std::unique_ptr<HttpClientBase> &client);
        static void NotAuthorized(std::unique_ptr<HttpClientBase> &client);

        static std::string GetMimeTypeByExtension(std::string extension);
};

#endif
