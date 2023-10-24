#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "core/HttpRequestHandler.hpp"
#include "core/HttpRequest.hpp"
#include "core/HttpResponse.hpp"
#include "core/RouteHandler.hpp"

#define MAX_BUFFER_SIZE 1024
#define MAX_EVENTS 64

class HttpServer {

private:
	// Port and Host Information
	int port;
	std::string host;

	// Configuration
	std::string configFilePath;

	// Socket Information
	int serverSocket;
	int maxConnections;

	// Routes and Handlers
	std::map<std::string, RouteHandler> routes;

	// Request and Response Buffers
	char buffer[MAX_BUFFER_SIZE];

	// Logging
	static bool enableLogging;


public:
	HttpServer();
	HttpServer(int port, const std::string& host);
	~HttpServer();
	HttpServer(HttpServer const &httpServer);
	HttpServer &operator=(HttpServer const &httpServer);

	int getPort();
	std::string getHost();
	int getMaxConnections();
	void setPort(int port);
	void setHost(const std::string& host);
	void setMaxConnections(int maxConnections);


	bool init();
	bool loadConfig(const std::string& configFilePath);
	void addRouteHandler(const RouteHandler routeHandler);
	const std::map<std::string, RouteHandler> getRouteHandlers();
	int acceptConnection();
	void handleRequest(int clientSocket);
	void processRequest();
	bool parseRequest(int clientSocket, char data[], HttpRequest &request);
	void sendResponse(int clientSocket, HttpResponse& response);
	void handleError(int clientSocket, int errorCode);
	void log(std::string message);
	void log(const std::string& message, int clientSocket);
	void run();
	void stop();
	//void log(int clientSocket, HttpRequest& request);
	void log(const std::string& message, int clientSocket, HttpRequest& request);
	//void serveStaticFile(int clientSocket, const std::string& filePath);
	//void executeCGI(int clientSocket, const std::string& cgiPath);
};