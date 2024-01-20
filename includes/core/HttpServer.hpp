#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <deque>
#include <time.h>

#include "core/HttpRequestHandler.hpp"
#include "core/HttpRequest.hpp"
#include "core/HttpResponse.hpp"
#include "core/RouteHandler.hpp"

#define MAX_EVENTS 64
#define MAX_BUFFER_SIZE 50000

class HttpServer {

private:
	// Port and Host Information
	int port;
	std::string host;
	ServerConfig &serverConfig;

	// Configuration
	std::string configFilePath;
	int clientBodySize;
	int fileBytes;

	// Socket Information
	int serverSocket;
	int maxConnections;

	// Routes and Handlers
	// TODO guardar servers em vez de endpoints
	std::map<std::string, RouteHandler> routes;

	// Request and Response Buffers
	//char requestBuffer[MAX_BUFFER_SIZE];
	static std::map<int, HttpServer*> _allActiveConnections;
	static std::map<int, time_t> _socketLastActiveTime;
	std::deque<int> serverActiveConnections;

	// Logging
	static bool enableLogging;
	std::deque<HttpResponse> responses;
	std::vector<int> errorCodes;


public:
	HttpServer(ServerConfig& serverConfig);
	HttpServer(int port, const std::string& host,ServerConfig& serverConfig);
	~HttpServer();
	HttpServer(HttpServer const &httpServer);
	HttpServer &operator=(HttpServer const &httpServer);

	int getPort();
	std::string getHost();
	int getServerSocket();
	int getMaxConnections();
	int getClientBodySize();
	std::vector<int> getErrorCodes();
	std::map<int, time_t>& getSocketLastActiveTime();
	ServerConfig getServerConfig();
	ssize_t getFileBytes();
	std::deque<HttpResponse>& getResponses();
	std::deque<int>& getServerActiveConnections();
	static std::string getFileContent(std::string filePath);

	void setErrorCodes(std::vector<int> errorCodes);
	void setPort(int port);
	void setHost(const std::string& host);
	void setMaxConnections(int maxConnections);
	void setClientBodySize(int clientBodySize);
	void setServerConfigs(ServerConfig serverConfigs);


	static void setupServers(std::vector<HttpServer> &servers);
	static void runServers(std::vector<HttpServer> &servers);
	static HttpServer* getServerFromClientSocket(int clientSocket);
	void acceptConnection(std::vector<pollfd> &pollSet);
	void closeConnection(int clientSocket, std::vector<pollfd> &pollSet);
	bool init();
	void handleReceive(int clientSocket, std::vector<pollfd> &pollSet);
	void handleSend(std::vector<pollfd> &pollSet);
	bool loadConfig(const std::string& configFilePath);
	void addRouteHandler(const RouteHandler routeHandler);
	const std::map<std::string, RouteHandler> getRouteHandlers();
	HttpResponse processRequest(char *dataBuffer, int clientSocket, HttpResponse &response);
	bool parseRequest(int clientSocket, char data[], HttpRequest &request, HttpResponse &response);
	void sendResponse(int clientSocket, HttpResponse& response);
	void handleError(int clientSocket, int errorCode);
	bool parseResource(const std::string& path, HttpRequest& request);
	void log(std::string message);
	void log(const std::string& message, int clientSocket);
	void run();
	void stop();
	//void log(int clientSocket, HttpRequest& request);
	void log(const std::string& message, int clientSocket, HttpRequest& request, HttpResponse& response);
	//void serveStaticFile(int clientSocket, const std::string& filePath);
	//void executeCGI(int clientSocket, const std::string& cgiPath);
};