#include "core/HttpServer.hpp"
#include "core/HttpStatusCode.hpp"
#include "core/Utils.hpp"

int epollFd = -1;
bool HttpServer::enableLogging = true;

HttpServer::HttpServer() {
}

HttpServer::HttpServer(int port, const std::string& host) {
	this->port = port;
	this->host = host;
}

HttpServer::~HttpServer() {
}

HttpServer::HttpServer(HttpServer const &httpServer) {
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

int HttpServer::getMaxConnections() {
	return this->maxConnections;
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

bool HttpServer::loadConfig(const std::string& configFilePath) {
	this->configFilePath = configFilePath;
	return true;
}

bool HttpServer::init() {
	// Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating server socket");
        return false;
    }

	// Set socket options to reuse address
    int reuseAddr = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) == -1) {
        perror("Error setting socket options");
        close(serverSocket);
        return false;
    }

	// Initialize server address structure
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(this->port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket to the server address
    if (bind(this->serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Error binding server socket");
        close(this->serverSocket);
        return false;
    }

	// Listen for incoming connections
    if (listen(serverSocket, maxConnections) == -1) {
        perror("Error listening for connections");
        close(serverSocket);
        return false;
    }

	// Set server socket to non-blocking mode
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1 || fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error setting server socket to non-blocking mode");
        close(serverSocket);
        return false;
    }

	// Initialize epoll
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        perror("Error creating epoll");
        close(serverSocket);
        return false;
    }

	// Add server socket to epoll
    struct epoll_event serverEvent;
    serverEvent.events = EPOLLIN | EPOLLET;  // Edge-triggered mode
    serverEvent.data.fd = serverSocket;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &serverEvent) == -1) {
        perror("Error adding server socket to epoll");
        close(serverSocket);
        close(epollFd);
        return false;
    }

    return true;
}

void HttpServer::run() {
	struct epoll_event events[MAX_EVENTS];

	std::cout << "Server listening on port " << this->port << std::endl;

	while (true) {
		// Wait for events to occur on any of the registered file descriptors
		int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);

		if (numEvents == -1) {
			perror("Error in epoll_wait");
			break;
		}

		for (int i = 0; i < numEvents; i++) {
			//std::cout << "Event on file descriptor " << events[i].data.fd << std::endl;
			if (events[i].data.fd == serverSocket) {
				// Handle incoming connection
				int clientSocket = acceptConnection();

				//std::cout << "Client socket: " << clientSocket << std::endl;

				if (clientSocket != -1) {
					// Set clientSocket to non-blocking mode
					int flags = fcntl(clientSocket, F_GETFL, 0);
					fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

					// Add the client socket to the epoll event list
					struct epoll_event clientEvent;
					clientEvent.events = EPOLLIN | EPOLLET;  // Edge-triggered mode
					clientEvent.data.fd = clientSocket;

					if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent) == -1) {
						perror("Error adding client socket to epoll");
						close(clientSocket);
					}
				}
			}
			else {
				// Handle data on client socket
				int clientSocket = events[i].data.fd;
				handleRequest(clientSocket);
			}
		}
	}

	// Clean up and close epoll file descriptor
	close(epollFd);
}

int HttpServer::acceptConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Error accepting connection");
        return -1;
    }

    //log("New connection accepted", clientSocket);

    return clientSocket;
}

void HttpServer::handleRequest(int clientSocket) {
    // Acknowledge the connection
    const char* acknowledgment = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    send(clientSocket, acknowledgment, std::strlen(acknowledgment), 0);

	// Buffer to read incoming data
    char dataBuffer[MAX_BUFFER_SIZE];

    // Read incoming data
    ssize_t bytesRead = recv(clientSocket, dataBuffer, sizeof(dataBuffer), 0);

    if (bytesRead == -1) {
        perror("Error reading data");
    }
	else if (bytesRead == 0) {
        std::cout << "Connection closed by client." << std::endl;
    }
	else {
		dataBuffer[bytesRead] = '\0';
        //std::cout << "Read " << bytesRead << " bytes of data." << std::endl;
		HttpStatusCode::setCurrentStatusCode("200");

		HttpRequest request;
		parseRequest(clientSocket, dataBuffer, request);

		std::cout << "-----------" << std::endl;
		Utils::printYellow("Request:");
		std::cout << dataBuffer << std::endl;

		HttpResponse response;
		routes[request.getResource()].handleRequest(request, response);
		log("Request received", clientSocket, request);
		std::cout << "-----------" << std::endl;
    }
    close(clientSocket);
}

bool HttpServer::parseRequest(int clientSocket, char data[], HttpRequest &request) {

	(void) clientSocket;

	if (!HttpRequest::isRequestValid(data)) {
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

	request.readHeaders(requestStream, request);

	// Parse the body
	std::string body = "";
	while (std::getline(requestStream, line) && !line.empty() && line != "\r") {
		body += line;
	}
	
	if (request.getHeaders().find("Content-Length") != request.getHeaders().end()) {
		size_t contentLength = std::atoi(request.getHeaders()["Content-Length"].c_str());

		if (contentLength != body.size()) {
			// set error
			HttpStatusCode::setCurrentStatusCode("400");
			return false;
		}
	}

	// Set the request properties
	request.setMethod(method);
	request.setResource(resource);
	request.setHttpVersion(httpVersion);
	request.setHost(request.getHeaders()["Host"]);
	request.setBody(body);

	// Print the request
	//std::cout << request << std::endl;

	return true;
}

void HttpServer::addRouteHandler(const RouteHandler routeHandler) {
	std::cout << "ADDED LOCATION: " << routeHandler.getLocation() << std::endl;
	routes[routeHandler.getLocation().getPath()] = routeHandler;
}

void HttpServer::sendResponse(int clientSocket, HttpResponse& response) {
	response.setStatusCode(HttpStatusCode::getCurrentStatusCode().first);
	response.setStatusMessage(HttpStatusCode::getCurrentStatusCode().second);
	std::string responseString = response.toString();
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

void HttpServer::log(const std::string& message, int clientSocket, HttpRequest& request) {
	(void) message;
	std::string statusCode = HttpStatusCode::getCurrentStatusCode().first;
	std::string statusMessage = HttpStatusCode::getCurrentStatusCode().second;
	
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
