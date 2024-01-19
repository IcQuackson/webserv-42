#include "core/HttpServer.hpp"
#include "core/HttpStatusCode.hpp"
#include "core/Utils.hpp"

int epollFd = -1;
bool HttpServer::enableLogging = true;
std::map<int, HttpServer*> HttpServer::_allActiveConnections = std::map<int, HttpServer*>();

HttpServer::HttpServer(ServerConfig& serverConfig): serverConfig(serverConfig) {
}

HttpServer::HttpServer(int port, const std::string& host, ServerConfig &serverConfig): serverConfig(serverConfig) {
	this->port = port;
	this->host = host;
	this->responses = std::deque<HttpResponse>();
}

HttpServer::~HttpServer() {
}

HttpServer::HttpServer(HttpServer const &httpServer): serverConfig(httpServer.serverConfig) {
	*this = httpServer;
}

HttpServer &HttpServer::operator=(HttpServer const &httpServer) {
	if (this != &httpServer) {
		this->port = httpServer.port;
		this->host = httpServer.host;
		this->configFilePath = httpServer.configFilePath;
		this->serverSocket = httpServer.serverSocket;
		this->maxConnections = httpServer.maxConnections;
		this->routes = httpServer.routes;
		this->clientBodySize = httpServer.clientBodySize;
		this->responses = httpServer.responses;
	}
	return *this;
}

int HttpServer::getPort() {
	return this->port;
}

const std::map<std::string, RouteHandler> HttpServer::getRouteHandlers() {
	return this->routes;
}

std::string HttpServer::getHost() {
	return this->host;
}

int HttpServer::getServerSocket() {
	return this->serverSocket;
}

int HttpServer::getMaxConnections() {
	return this->maxConnections;
}

int HttpServer::getClientBodySize() {
	return this->clientBodySize;
}

ssize_t HttpServer::getFileBytes() {
	return this->fileBytes;
}

std::deque<int>& HttpServer::getServerActiveConnections() {
	return this->serverActiveConnections;
}

std::deque<HttpResponse>& HttpServer::getResponses() {
	return this->responses;
}
ServerConfig HttpServer::getServerConfig() {
	return this->serverConfig;
}

void HttpServer::setPort(int port) {
	this->port = port;
}

void HttpServer::setHost(const std::string& host) {
	this->host = host;
}

void HttpServer::setMaxConnections(int maxConnections) {
	this->maxConnections = maxConnections;
}

void HttpServer::setClientBodySize(int clientBodySize) {
	this->clientBodySize = clientBodySize;
}

void HttpServer::setServerConfigs(ServerConfig serverConfig) {
	this->serverConfig = serverConfig;
}

bool HttpServer::loadConfig(const std::string& configFilePath) {
	this->configFilePath = configFilePath;
	return true;
}

void HttpServer::setupServers(std::vector<HttpServer> &servers) {
	// Initialize all servers
	for (size_t i = 0; i < servers.size(); ++i) {
        if (!servers[i].init()) {
            std::cerr << "Failed to initialize server on port " << servers[i].getPort() << std::endl;
            exit(1);
        }
		std::cout << "Server initialized on port " << servers[i].getPort() << std::endl;
		std::cout << "Server socket: " << servers[i].getServerSocket() << std::endl;
    }
}

bool HttpServer::init() {
	// Create a socket
	this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == -1) {
		perror("Error creating server socket");
		return false;
	}

	std::cout << "Server socket created: " << serverSocket << std::endl;

	// Set socket options to reuse address
	int reuseAddr = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) == -1) {
		perror("Error setting socket options");
		close(serverSocket);
		return false;
	}

	// Initialize server address structure
	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	// Bind the socket to the server address
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("Error binding server socket");
		close(serverSocket);
		return false;
	}

	// Listen for incoming connections
	if (listen(serverSocket, SOMAXCONN) == -1) {
		perror("Error listening for connections");
		close(serverSocket);
		return false;
	}

	std::cout << "Server listening on port " << port << std::endl;

	// Set server socket to non-blocking mode
	int flags = fcntl(serverSocket, F_GETFL, 0);
	if (flags == -1 || fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("Error setting server socket to non-blocking mode");
		close(serverSocket);
		return false;
	}

	return true;
}

void printPollSet(std::vector<pollfd> &pollSet) {
	std::cout << "Poll set: " << std::endl;
	for (size_t i = 0; i < pollSet.size(); i++) {
		std::cout << "fd: " << pollSet[i].fd << std::endl;
		std::cout << "events: " << pollSet[i].events << std::endl;
		std::cout << "revents: " << pollSet[i].revents << std::endl;
	}
}

HttpServer* HttpServer::getServerFromClientSocket(int clientSocket) {
	return _allActiveConnections.at(clientSocket);
}

void HttpServer::runServers(std::vector<HttpServer> &servers) {
	std::vector<pollfd> pollSet(servers.size());

	std::cout << "Running servers" << std::endl;

    // Populate the poll set
    for (size_t i = 0; i < servers.size(); i++) {
        pollSet[i].fd = servers[i].getServerSocket();
        pollSet[i].events = POLLIN | POLLOUT;
        pollSet[i].revents = 0;
    }

	printPollSet(pollSet);

    // Main event loop
    while (true) {
        int numReady = poll(&pollSet[0], pollSet.size(), 200);
        if (numReady == -1) {
            perror("Error in poll");
            exit(1);
        }

        // Handle events for each server
        for (size_t i = 0; i < pollSet.size(); i++) {
			if (pollSet[i].revents & POLLIN) {
				if (i < servers.size() && servers[i].getServerSocket() == pollSet[i].fd) {
					servers[i].acceptConnection(pollSet);
				}
				else {
					getServerFromClientSocket(pollSet[i].fd)->handleReceive(pollSet[i].fd, pollSet);
				}
			}
			// Check for errors 
            else if (pollSet[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "Error on fd " << pollSet[i].fd << std::endl;
				HttpServer *server = getServerFromClientSocket(pollSet[i].fd);
				server->closeConnection(pollSet[i].fd, pollSet);
            }
			 // Check if a client socket is ready to write
			else if (pollSet[i].revents & POLLOUT) {
				HttpServer *server = getServerFromClientSocket(pollSet[i].fd);
				server->handleSend(pollSet);
				pollSet[i].events &= ~POLLOUT;
			}
        }
    }
}

void HttpServer::acceptConnection(std::vector<pollfd> &pollSet) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

	// Accept the client connection
    int clientSocket = accept(this->serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        perror("Error accepting connection");
        return;
    }
	std::cout << "Connection accepted on socket " << clientSocket << std::endl;

    // Add the new client socket to the active connections
    _allActiveConnections.insert(std::pair<int, HttpServer*>(clientSocket, this));
	serverActiveConnections.push_back(clientSocket);

    // Add the new client socket to the poll set
    struct pollfd pfd;
    pfd.fd = clientSocket;
    pfd.events = POLLIN | POLLOUT;
    pollSet.push_back(pfd);
}

void HttpServer::closeConnection(int clientSocket, std::vector<pollfd> &pollSet) {
	// Close the client socket
	close(clientSocket);

	// Remove the client socket from the poll set
	for (size_t i = 0; i < pollSet.size(); i++) {
		if (pollSet[i].fd == clientSocket) {
			pollSet.erase(pollSet.begin() + i);
			break;
		}
	}

	// Remove the client socket from the active connections
	_allActiveConnections.erase(clientSocket);

	// erase from serverActiveConnections
	serverActiveConnections.erase(std::remove(serverActiveConnections.begin(), serverActiveConnections.end(), clientSocket), serverActiveConnections.end());
}

void HttpServer::handleReceive(int clientSocket, std::vector<pollfd> &pollSet) {

	// Receive the HTTP request
	char requestBuffer[MAX_BUFFER_SIZE];
	memset(requestBuffer, 0, MAX_BUFFER_SIZE);
	ssize_t bytesRead = recv(clientSocket, requestBuffer, MAX_BUFFER_SIZE - 1, 0);
	requestBuffer[MAX_BUFFER_SIZE - 1] = '\0';
	if (bytesRead == -1) {
		perror("Error receiving data");
		closeConnection(clientSocket, pollSet);
		return;
	}

	// Process the HTTP request and generate a response
	HttpResponse response;
	processRequest(requestBuffer, clientSocket, response);
	this->responses.push_back(response);

}

void HttpServer::handleSend(std::vector<pollfd> &pollSet) {
    if (_allActiveConnections.empty() || serverActiveConnections.empty() || responses.empty()) {
        // No active connections or no responses to send
        return;
    }

    // Get the first active connection and response
    int clientSocket = serverActiveConnections.front();
    std::string response = this->responses.front().toString();

    // Send the HTTP response
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
    if (bytesSent == -1) {
        perror("Error sending data");
        closeConnection(clientSocket, pollSet);
        return;
    }

	// Remove the response from the queue
    responses.erase(responses.begin());

    // Close the client socket
    closeConnection(clientSocket, pollSet);
}

bool isBufferEmpty(const char* buffer) {
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (buffer[i] != 0) {
            return false;  // Buffer is not empty
        }
    }
    return true;  // Buffer is empty
}

HttpResponse HttpServer::processRequest(char *dataBuffer, int clientSocket, HttpResponse &response) {
    // Acknowledge the connection
    //const char* acknowledgment = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";

    //ssize_t bytesRead;

	// Read data from the client
	std::cout << "clientSocket: " << clientSocket << std::endl;
	//bytesRead = std::strlen(dataBuffer);

	// TODO: store bytes read

    
	//std::cout << "Read " << bytesRead << " bytes of data." << std::endl;
	response.setStatusCode("200");
	std::cout << "-----------" << std::endl;
	Utils::printYellow("Request:");
	HttpRequest request;
	bool isValidRequest = parseRequest(clientSocket, dataBuffer, request, response);

	if (isValidRequest) {
		// Check if the requested resource exists
		std::cout << "request.getResource(): " << request.getResource() << std::endl;
		if (!parseResource(request.getResource(), request)) {
			response.setStatusCode("404");
			response.setBody("Resource does not exist");
				response.setDefaultErrorPage(404);
		}
		else {
				std::string redirect_path = routes[request.getRoute()].getLocation().getRedirection();
				if (!redirect_path.empty()) {
					if (routes.find(redirect_path) != routes.end())
					{
						request.setRoute(redirect_path);
						routes[redirect_path].handleRequest(request, response);
					}
					else
					{
						response.setStatusCode("404");
						response.setBody("Resource does not exist");
					}
				}
				else
				routes[request.getRoute()].handleRequest(request, response);
		}
	}
	log("Request received", clientSocket, request, response);
	std::cout << "-----------" << std::endl;
	std::cout << std::endl;

	return response;
}

bool read_request_content(std::string &input)
{
	std::istringstream iss(input);
    std::string token;
    std::string separator = "\r";  // Change this to your desired separator

    size_t pos = 0;
    while ((pos = input.find(separator)) != std::string::npos) {
        // Extract the token before the separator
        token = input.substr(0, pos);
        // Remove the processed token and the separator from the input
        input.erase(pos, separator.length());
		return (0);
    }
	return (1);
}

bool HttpServer::parseRequest(int clientSocket, char data[], HttpRequest &request, HttpResponse& response) {

	(void) clientSocket;

	if (!HttpRequest::isRequestValid(data, response)) {
		return false;
	}

	std::string dataString(data);

	// Parse the request
	std::istringstream requestStream(dataString);
	std::string line;

	// Parse the request line
	std::getline(requestStream, line);
	std::istringstream requestLineStream(line);
	std::string method;
	std::string resource;
	std::string httpVersion;
	requestLineStream >> method >> resource >> httpVersion;
	
	request.readArgs(resource);

	resource = resource.substr(0, resource.find("?"));

	//request.readHeaders(requestStream, request);
	if (!request.readHeaders(requestStream, request, response)) {
		return false;
	}

	bool is_file = 0;
	
	if (request.getHeaders().find("Content-Type") != request.getHeaders().end())
	{
		std::map<std::string, std::string> headers = request.getHeaders();
		std::map<std::string, std::string>::iterator it = headers.find("Content-Type");
		if (it != headers.end() && it->second.find("boundary") != std::string::npos)
		{
			is_file = 1;
			size_t pos = it->second.find(';');
			it->second = it->second.substr(0, pos);
			std::getline(requestStream, line);
			if (!request.readHeaders(requestStream, request, response))
				return false;
		}
	}

	// Parse the body
	std::string body = "";
	int flag = 0;
	int count_lines = 0;
	while (std::getline(requestStream, line) && line != "\r") {
		if (is_file && flag)
			body += '\n';
		if (is_file && !read_request_content(line))
		{
			body += line;
			break;
		}
		body += line;
		if (flag)
			count_lines++;
		flag = 1;
	}

	/* if (is_file == 0)
		this->fileBytes = (body.size() + count_lines); */


	if (request.getHeaders().find("Content-Length") != request.getHeaders().end()) {
		//ssize_t contentLength = std::atoi(request.getHeaders()["Content-Length"].c_str());

		// TODO: check if contentLength is valid

		/* if (contentLength != this->fileBytes) {
			std::cout << "Error: Content-Length does not match body size" << std::endl;
			response.setStatusCode("400");
			return false;
		} */
	} 

	// Set the request properties
	request.setMethod(method);
	request.setResource(resource);
	request.setHttpVersion(httpVersion);
	request.setHost(request.getHeaders()["Host"]);
	request.setBody(body);

	std::cout << "BODY" << std::endl;
	std::cout << body << std::endl;

	// Print the request
	//std::cout << request << std::endl;

	return true;
}

void HttpServer::addRouteHandler(const RouteHandler routeHandler) {
	routes[routeHandler.getLocation().getPath()] = routeHandler;
	std::cout << "Route added: " << routeHandler.getLocation().getPath() << std::endl;
	std::cout << "Location added: " <<routeHandler.getLocation() << std::endl;
}

void HttpServer::sendResponse(int clientSocket, HttpResponse& response) {
	response.setStatusMessage(HttpStatusCode::getHttpStatusCode(response.getStatusCode()));
	std::string responseString = response.toString();

	std::cout << "Response: " << std::endl << responseString << std::endl;
	send(clientSocket, responseString.c_str(), responseString.size(), 0);
}

void HttpServer::log(std::string message) {

	if (!enableLogging) {
		return;
	}

    // Get current time
    std::time_t currentTime = std::time(0);
    std::tm* now = std::localtime(&currentTime);

    // Format time as HH:MM:SS
    std::ostringstream timeString;
    timeString << std::setfill('0') << std::setw(2) << now->tm_hour << ":"
               << std::setfill('0') << std::setw(2) << now->tm_min << ":"
               << std::setfill('0') << std::setw(2) << now->tm_sec;

    // Print log message with timestamp
    std::cout << "[" << timeString.str() << "] " << message << std::endl;
}

/* void HttpServer::log(const std::string& message, int clientSocket) {

    if (!enableLogging) {
        return;
    }

    // Get current time
    std::time_t currentTime = std::time(0);
    std::tm* now = std::localtime(&currentTime);

    // Format time as HH:MM:SS
    std::ostringstream timeString;
    timeString << std::setfill('0') << std::setw(2) << now->tm_hour << ":"
               << std::setfill('0') << std::setw(2) << now->tm_min << ":"
               << std::setfill('0') << std::setw(2) << now->tm_sec;

    // Print log message with timestamp and socket information
    std::cout << "[" << timeString.str() << "] [Socket " << clientSocket << "] " << message << std::endl;
} */

void HttpServer::log(const std::string& message, int clientSocket, HttpRequest& request, HttpResponse& response) {
	(void) message;
	std::string statusCode = response.getStatusCode();
	std::string statusMessage = HttpStatusCode::getHttpStatusCode(statusCode);
	
	std::cout << this->getHost() << ":" << this->getPort() << " - Socket:" << clientSocket << " - ";
	
	// Check if status code indicates an error (4xx or 5xx)
	if (statusCode[0] == '4' || statusCode[0] == '5') {
		Utils::printRed(statusCode + " " + statusMessage);
		return;
	}
	// Print first line of request.getbody()
	std::string logInfo = request.getMethod() + " " + request.getResource()\
				  + " " + request.getHttpVersion() + " " + statusCode\
				  + " " + statusMessage;

	// Check if status code indicates success (2xx), informational (1xx), or a redirect (3xx)
	if (statusCode[0] == '2') {
		// Print status code in green
		Utils::printGreen(logInfo);
	}
	else if (statusCode[0] == '1' || statusCode[0] == '3') {
		// Print status code in yellow for informational and redirect
		Utils::printYellow(logInfo);
	}
	else {
		Utils::printRed(logInfo);
	}
}

bool HttpServer::parseResource(const std::string& path, HttpRequest& request) {
	std::string routePath = path;
	std::string requestedPath = "";

	while (!routePath.empty()) {
		std::cout << "Requested path: " << requestedPath << std::endl;
		// Check if the current path is a valid endpoint
		if (routes.find(routePath) != routes.end()) {
			request.setRoute(routePath);
			request.setResource(requestedPath);
			std::cout << "Endpoint found: " << routePath << std::endl;
			std::cout << "Requested path: " << requestedPath << std::endl;
			return true;
		}

		// Remove the last slash + string combination
		std::size_t lastSlash = routePath.find_last_of('/');
		if (lastSlash == std::string::npos) {
			break;  // No more parts to check
		}
		requestedPath = "/" + routePath.substr(lastSlash + 1) + requestedPath;
		routePath = routePath.substr(0, lastSlash);
	}

	return false;
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& request) {
	os << "Method: " << request.getMethod() << std::endl;
	os << "Resource: " << request.getResource() << std::endl;
	os << "HTTP Version: " << request.getHttpVersion() << std::endl;
	os << "Host: " << request.getHost() << std::endl;

	os << "Headers: " << std::endl;
	const std::map<std::string, std::string>& headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		os << "-> " << it->first << ": " << it->second << std::endl;
	}
	os << "Body: " << request.getBody() << std::endl;

	os << "Args: " << std::endl;
	const std::map<std::string, std::string>& args = request.getArgs();
	for (std::map<std::string, std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
		os << "-> " << it->first << ": " << it->second << std::endl;
	}
	return os;
}
