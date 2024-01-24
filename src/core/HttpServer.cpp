#include "core/HttpServer.hpp"
#include "core/HttpStatusCode.hpp"
#include "core/Utils.hpp"
#include "core/MultiPartRequest.hpp"

int epollFd = -1;
bool HttpServer::enableLogging = true;
std::vector<pollfd> HttpServer::_pollSet = std::vector<pollfd>();
std::map<int, HttpServer*> HttpServer::_allActiveConnections = std::map<int, HttpServer*>();
std::map<int, time_t> HttpServer::_socketLastActiveTime = std::map<int, time_t>();

HttpServer::HttpServer(ServerConfig& serverConfig): serverConfig(serverConfig) {
}

HttpServer::HttpServer(int port, const std::string& host, ServerConfig &serverConfig): serverConfig(serverConfig) {
	this->port = port;
	this->host = host;
	this->responses = std::map<int, HttpResponse>();
	setErrorCodes(serverConfig.getError_codes());
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

const std::map<int, HttpResponse>& HttpServer::getResponses() {
	return this->responses;
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

std::map<int, time_t>& HttpServer::getSocketLastActiveTime() {
	return this->_socketLastActiveTime;
}

ServerConfig HttpServer::getServerConfig() {
	return this->serverConfig;
}

std::vector<int> HttpServer::getErrorCodes() {
	return this->errorCodes;
}

void HttpServer::setErrorCodes(std::vector<int> errorCodes) {
	for (size_t i = 0; i < errorCodes.size(); i++) {
		this->errorCodes.push_back(errorCodes[i]);
	}
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

void HttpServer::setupServers(std::vector<HttpServer*> &servers) {
	// Initialize all servers
	for (size_t i = 0; i < servers.size(); i++) {
		if (!servers[i]->init()) {
			throw std::runtime_error("Error initializing server");
		}
		std::cout << "Server initialized on port " << servers[i]->getPort() << std::endl;
		std::cout << "Server socket: " << servers[i]->getServerSocket() << std::endl;
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

	return true;
}

void printPollSet(std::vector<pollfd> &_pollSet) {
	std::cout << "Poll set: " << std::endl;
	for (size_t i = 0; i < _pollSet.size(); i++) {
		std::cout << "fd: " << _pollSet[i].fd << std::endl;
		std::cout << "events: " << _pollSet[i].events << std::endl;
		std::cout << "revents: " << _pollSet[i].revents << std::endl;
	}
}

HttpServer* HttpServer::getServerFromClientSocket(int clientSocket) {
	return _allActiveConnections.at(clientSocket);
}

void HttpServer::closeIfTimedOut(int clientSocket) {
	if (_socketLastActiveTime.find(clientSocket) == _socketLastActiveTime.end()) {
		return;
	}

	time_t last_request_time = _socketLastActiveTime[clientSocket];
	if ((std::time(0) - last_request_time) > 4) {
		std::cout << "Connection closed by timeout" << std::endl;
		HttpServer *server = getServerFromClientSocket(clientSocket);
		server->closeConnection(clientSocket);
		return;
	}
}

void HttpServer::runServers(std::vector<HttpServer*> &servers) {
	for (size_t i = 0; i < servers.size(); i++) {
		_pollSet.push_back(pollfd());
	}

	std::cout << "Running servers" << std::endl;

	// Populate the poll set
	for (size_t i = 0; i < servers.size(); i++) {
		_pollSet[i].fd = servers[i]->getServerSocket();
		_pollSet[i].events = POLLIN;
		_pollSet[i].revents = 0;
	}

	//printPollSet(_pollSet);

	// Main event loop
	while (true) {
		int numReady = poll(&_pollSet[0], _pollSet.size(), 200);
		if (numReady == -1) {
			throw std::runtime_error("Error in poll");
		}

		// Handle events for each server
		for (size_t i = 0; i < _pollSet.size(); i++) {

			closeIfTimedOut(_pollSet[i].fd);
			// Check if a socket is ready to read
			if (_pollSet[i].revents & POLLIN) {
				// Check if a new client connection is ready to be accepted
				if (i < servers.size() && servers[i]->getServerSocket() == _pollSet[i].fd) {
					std::cout << "Accepting connection on fd " << _pollSet[i].fd << std::endl;
					servers[i]->acceptConnection();
				}
				// Check if a client socket is ready to read
				else {
					_socketLastActiveTime[_pollSet[i].fd] = std::time(0);
					std::cout << "Received data on fd " << _pollSet[i].fd << std::endl;
					getServerFromClientSocket(_pollSet[i].fd)->handleReceive(_pollSet[i].fd);
				}
			}
			// Check for errors 
			else if (_pollSet[i].revents & (POLLERR | POLLHUP | POLLNVAL | POLLRDHUP)) {
				printPollSet(_pollSet);
				std::cerr << "Error on fd " << _pollSet[i].fd << std::endl;
				HttpServer *server = getServerFromClientSocket(_pollSet[i].fd);
				server->closeConnection(_pollSet[i].fd);
			}
			 // Check if a socket is ready to write
			else if (_pollSet[i].revents & POLLOUT) {
				std::cout << "Sending data on fd " << _pollSet[i].fd << std::endl;
				HttpServer *server = getServerFromClientSocket(_pollSet[i].fd);
				server->handleSend(_pollSet[i].fd);
				_pollSet[i].revents &= ~POLLOUT;
				//_pollSet[i].events &= ~POLLOUT;
			}
		}
	}
}

void HttpServer::acceptConnection() {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);

	// Accept the client connection
	int clientSocket = accept(this->serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
	if (clientSocket == -1) {
		perror("Error accepting connection");
		closeConnection(clientSocket);
		return;
	}
	int flags = fcntl(clientSocket, F_GETFL, 0);
	if (flags == -1 || fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("Error setting server socket to non-blocking mode");
		closeConnection(clientSocket);
		return;
	}
	std::cout << "Connection accepted on socket " << clientSocket << std::endl;

	// Add the new client socket to the active connections
	_allActiveConnections.insert(std::pair<int, HttpServer*>(clientSocket, this));
	_socketLastActiveTime.insert(std::pair<int, time_t>(clientSocket, time(0)));
	serverActiveConnections.push_back(clientSocket);

	// Add the new client socket to the poll set
	struct pollfd pfd;
	pfd.fd = clientSocket;
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
	_pollSet.push_back(pfd);
}

void HttpServer::closeConnection(int clientSocket) {
	// Close the client socket
	close(clientSocket);

	// Remove the client socket from the poll set
	for (size_t i = 0; i < _pollSet.size(); i++) {
		if (_pollSet[i].fd == clientSocket) {
			_pollSet.erase(_pollSet.begin() + i);
			break;
		}
	}

	// Remove the client socket from the active connections
	_allActiveConnections.erase(clientSocket);
	_socketLastActiveTime.erase(clientSocket);
	serverActiveConnections.erase(std::remove(serverActiveConnections.begin(), serverActiveConnections.end(), clientSocket), serverActiveConnections.end());
	responses.erase(clientSocket);
}

void HttpServer::handleReceive(int clientSocket) {

	static std::map<int, MultiPartRequest> clientBuffers = std::map<int, MultiPartRequest>();

	// Receive the HTTP request
	char requestBuffer[MAX_BUFFER_SIZE];
	memset(requestBuffer, 0, MAX_BUFFER_SIZE);
	ssize_t bytesRead = recv(clientSocket, requestBuffer, MAX_BUFFER_SIZE - 1, 0);
	requestBuffer[MAX_BUFFER_SIZE - 1] = '\0';
	if (bytesRead < 0) {
		perror("Error receiving data");
		closeConnection(clientSocket);
		return;
	}
	else if (bytesRead == 0) {
		std::cout << "Connection closed by client" << std::endl;
		closeConnection(clientSocket);
		return;
	}
	std::cout << "Raw data received: " << std::endl << requestBuffer << std::endl;

	std::string dataString(requestBuffer);
	// Check if the request is multipart/form-data
	if (dataString.find("Content-Type: multipart/form-data") != std::string::npos
		&& clientBuffers.find(clientSocket) == clientBuffers.end()) {

		MultiPartRequest multiPartRequest;
		multiPartRequest.setData(dataString);
		multiPartRequest.setBoundary(MultiPartRequest::getBoundary(dataString));
		std::string boundary = multiPartRequest.getBoundary();

		// Check if the boundary is complete
		if (multiPartRequest.getData().find(boundary + "--\r\n") == std::string::npos) {
			std::cout << "Boundary not complete" << std::endl;
			clientBuffers.insert(std::pair<int, MultiPartRequest>(clientSocket, multiPartRequest));
			return;
		}
	}
	// Check if the request is multipart/form-data and the boundary is not complete
	else if (clientBuffers.find(clientSocket) != clientBuffers.end()) {

		clientBuffers[clientSocket].appendData(dataString);
		std::string boundary = clientBuffers[clientSocket].getBoundary();

		// Check if the boundary is complete
		if (clientBuffers[clientSocket].getData().find(boundary + "--\r\n") == std::string::npos) {
			return;
		}
		else {
			std::cout << "Buffer Deleted" << std::endl;
			dataString = clientBuffers[clientSocket].getData();
			clientBuffers.erase(clientSocket);
		}
	}
	std::cout << "Data received: " << std::endl << dataString << "END OF DATA" << std::endl;

	// Process the HTTP request and generate a response
	HttpResponse response;
	response.setErrorCodes(getServerConfig().getError_codes());
	
	processRequest(dataString, clientSocket, response);

	int statusCodeInt;

	std::stringstream ss(response.getStatusCode());
	ss >> statusCodeInt;

	for (size_t i = 0; i < serverConfig.getError_codes().size(); i++) {
		if (serverConfig.getError_codes()[i] == statusCodeInt) {
			RouteHandler::handleRegularFile(getServerConfig().getErrorPage(), response);
		}
	}
	this->responses[clientSocket] = response;
}

std::string HttpServer::getFileContent(std::string filePath) {
		std::ifstream fileStream(filePath.c_str(), std::ios::binary);

	if (!fileStream) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return "";
	}

	std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	std::cout << "File content: " << std::endl << fileContent << std::endl;
	fileStream.close();

	return fileContent;
}

void HttpServer::handleSend(int clientSocket) {
	if (serverActiveConnections.empty() || responses.empty()) {
		std::cout << "No active connections or responses" << std::endl;
		return;
	}

	if (responses.find(clientSocket) == responses.end()) {
		std::cout << "No response for client socket: " << clientSocket << std::endl;
		return;
	}

	// Get the first active connection and response
	std::string response = responses[clientSocket].toString();

	// Send the HTTP response
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
	if (bytesSent < 0) {
		perror("Error sending data");
		closeConnection(clientSocket);
		return;
	}
	else if (bytesSent == 0) {
		std::cout << "Connection closed by client" << std::endl;
		closeConnection(clientSocket);
		return;
	}

	// Close the client socket
	closeConnection(clientSocket);
}

bool isBufferEmpty(const char* buffer) {
	for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
		if (buffer[i] != 0) {
			return false;  // Buffer is not empty
		}
	}
	return true;  // Buffer is empty
}

bool HttpServer::isHostNameAllowed(std::string target) {
	std::cout << "Host wanted: " << target << std::endl;
	std::cout << "Server hostnames:" << std::endl;

	std::vector<std::string> hostNames = serverConfig.getServer_names();
	for (size_t i = 0; i < hostNames.size(); i++) {
		std::cout << hostNames[i] << std::endl;
	}

	int portInt = this->getPort();
	std::stringstream ss;
	ss << portInt;
	std::string port = ss.str();

	if (target.find(':') != std::string::npos) {
		std::string host = target.substr(0, target.find(':'));
		if (host == "localhost") {
			// Replace host part with 127.0.0.1
			target = "127.0.0.1" + target.substr(target.find(':'));
		}
	}

	std::cout << "Server host: " << getHost() << ":" << port << std::endl;
	// Check if the target is in the list of hostnames
	return std::find(hostNames.begin(), hostNames.end(), target) != hostNames.end()
		|| target == getHost() + ":" + port;
}

HttpResponse HttpServer::processRequest(std::string data, int clientSocket, HttpResponse &response) {
	// Read data from the client
	std::cout << "clientSocket: " << clientSocket << std::endl;

	// TODO: store bytes read
	
	response.setStatusCode("200");
	std::cout << "-----------" << std::endl;
	Utils::printYellow("Request:");
	HttpRequest request;
	bool isValidSyntax = parseRequest(clientSocket, data, request, response);

	if (!isValidSyntax) {
		std::cerr << "Invalid syntax" << std::endl;
		return response;
	}
	// Check if the requested resource exists
	std::cout << "Resource wanted: " << request.getResource() << std::endl;
	if (!parseResource(request.getResource(), request)) {
		response.setStatusCode("404");
		std::cerr << "Resource not found" << std::endl;
		return response;
	}
	// TODO: make server config store all the hosts
	if (!isHostNameAllowed(request.getHost())) {
		std::cerr << "Host not allowed" << std::endl;
		response.setStatusCode("403");
		return response;
	}
	else {
		std::string redirect_path = routes[request.getRoute()].getLocation().getRedirection();
		if (!redirect_path.empty() && routes.find(redirect_path) != routes.end()) {
			request.setRoute(redirect_path);
			routes[redirect_path].handleRequest(request, response);
		}
		else if (!redirect_path.empty()) {
			response.setStatusCode("404");
		}
		else {
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

bool HttpServer::parseRequest(int clientSocket, std::string data, HttpRequest &request, HttpResponse& response) {

	(void) clientSocket;

	if (!HttpRequest::isRequestValid(data, response)) {
		return false;
	}

	// Parse the request
	std::istringstream requestStream(data);
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

	if (!request.readHeaders(requestStream, request, response)) {
		std::cerr << "Error reading headers 1" << std::endl;
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
			if (!request.readHeaders(requestStream, request, response)) {
				std::cerr << "Error reading headers 2" << std::endl;
				return false;
			}
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
	return true;
}

std::string HttpServer::trim(const std::string& str) {
	// Find the first non-space character
	std::string::const_iterator first = str.begin();
	while (first != str.end() && std::isspace(*first)) {
		++first;
	}

	// Find the last non-space character
	std::string::const_reverse_iterator last = str.rbegin();
	while (last != str.rend() && std::isspace(*last)) {
		++last;
	}

	// Check if the string is empty or contains only spaces
	if (first == str.end() || last.base() == str.begin()) {
		return std::string();  // Return an empty string
	}
	else {
		return std::string(first, last.base());
	}
}

void HttpServer::addRouteHandler(RouteHandler &routeHandler) {
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

	std::cout << "Route path: " << routePath << std::endl;

	std::cout << "Path: " << path << std::endl;
	std::cout << "Print all endpoints: " << std::endl;
	for (std::map<std::string, RouteHandler>::iterator it = routes.begin(); it != routes.end(); ++it) {
		std::cout << it->first << std::endl;
	}

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
		if (lastSlash == 0 && routes.find("/") != routes.end()) {
			routePath = "/";
		}
		else {
			routePath = routePath.substr(0, lastSlash);
		}
		std::cout << "Route path: " << routePath << std::endl;
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
