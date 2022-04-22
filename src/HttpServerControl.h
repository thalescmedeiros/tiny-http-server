#ifndef H_HttpServerControl
#define H_HttpServerControl

#include <vector>
#include <memory>
#include <vector>
#include <string>
#include "GlobalSettings.h"
#include "HttpServer.h"
#include "HttpServerApp.h"
#include "HttpProcessor.h"

#define EVENT_WS_USERS_CHANGED 			101

class HttpClient;

class HttpServerControl
{
	public:
		HttpServerControl();
		~HttpServerControl();
		void Initialize();
		bool AddClient(std::unique_ptr<HttpClient> &client);
		void Tick();
		void RemoveClient(std::unique_ptr<HttpClient> &client);
		int GetClientsConnected() { return clients.size(); }
		void SendTextToWSClients(int eventType, std::string textOut);
		bool HasWebSocketMessageToSend(int &eventType, std::string &textOut);
		std::string GetOnlineUsersJson();

		std::unique_ptr<HttpProcessor> &GetProcessor() { return processor; }
		std::unique_ptr<HttpServerApp> &GetApp() { return serverApp; }

	private:
		std::unique_ptr<HttpServerControl> serverControl;
		std::unique_ptr<HttpServerApp> serverApp;
		std::unique_ptr<HttpServer> server;
		std::unique_ptr<HttpProcessor> processor;
		std::vector<std::unique_ptr<HttpClient>> clients;
		std::vector<int> messagesToClientWSEventType;
		std::vector<std::string> messagesToClientWSText;
};

#endif
