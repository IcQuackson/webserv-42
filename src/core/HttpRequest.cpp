#include "core/HttpRequest.hpp"
#include "core/HttpServer.hpp"
#include "core/HttpStatusCode.hpp"
#include "core/HttpResponse.hpp"

HttpRequest::HttpRequest() {
	args = std::map<std::string, std::string>();
	headers = std::map<std::string, std::string>();
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &httpRequest) {
    method = httpRequest.method;
    resource = httpRequest.resource;
	route = httpRequest.route;
    httpVersion = httpRequest.httpVersion;
    host = httpRequest.host;
    args = httpRequest.args;
	headers = httpRequest.headers;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &httpRequest) {
    if (this != &httpRequest) {
        method = httpRequest.method;
        resource = httpRequest.resource;
		route = httpRequest.route;
        httpVersion = httpRequest.httpVersion;
        host = httpRequest.host;
        args = httpRequest.args;
		headers = httpRequest.headers;
    }
    return *this;
}

std::string HttpRequest::getRoute() const {
	return route;
}

void HttpRequest::setRoute(const std::string& newRoute) {
	route = newRoute;
}

std::string HttpRequest::getMethod() const {
	return method;
}

void HttpRequest::setMethod(const std::string& newMethod) {
	method = HttpServer::trim(newMethod);
}

std::string HttpRequest::getResource() const {
	return resource;
}

void HttpRequest::setResource(const std::string& newResource) {
	resource = HttpServer::trim(newResource);
}

std::string HttpRequest::getHttpVersion() const {
	return httpVersion;
}

void HttpRequest::setHttpVersion(const std::string& newHttpVersion) {
	httpVersion = HttpServer::trim(newHttpVersion);
}

std::string HttpRequest::getHost() const {
	return host;
}

void HttpRequest::setHost(const std::string& newHost) {
	host = HttpServer::trim(newHost);
}

std::map<std::string, std::string> HttpRequest::getArgs() const {
	return args;
}

void HttpRequest::setArgs(const std::map<std::string, std::string> newArgs) {
	args = newArgs;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const {
	return headers;
}

void HttpRequest::setHeaders(const std::map<std::string, std::string>& newHeaders) {
	headers = newHeaders;
}

std::string HttpRequest::getBody() const {
	return this->body;
}

void HttpRequest::setBody(const std::string& newBody) {
	this->body = newBody;
}

void HttpRequest::setQueryString(const std::string& query) {
	this->queryString = query;
}


std::string HttpRequest::getQueryString() const{
	return this->queryString;
}


bool HttpRequest::readArgs(const std::string& path) {
	std::string argString;

	(void) args;

	// Find the position of the first question mark
	std::size_t pos = path.find("?");
	if (pos == std::string::npos) {
		return false;
	}

	// Get the argument string
	argString = path.substr(pos + 1);
	this->queryString = argString;

	// Parse the argument string
	std::istringstream argStream(argString);
	std::string argPair;
	while (std::getline(argStream, argPair, '&')) {
		std::istringstream argPairStream(argPair);
		std::string argName;
		std::getline(argPairStream, argName, '=');
		std::string argValue;
		std::getline(argPairStream, argValue);
		this->args[argName] = argValue;
	}

	return true;
}

// needs request argument to set status code
bool HttpRequest::readHeaders(std::istringstream& requestStream, HttpRequest& request, HttpResponse& response) {
    std::string line;

    while (std::getline(requestStream, line) && line != "\r") {
        std::istringstream headerLineStream(line);
        std::string headerName;
        std::getline(headerLineStream, headerName, ':');
        std::string headerValue;
        std::getline(headerLineStream, headerValue);
        request.headers[headerName] = headerValue;
		//std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa:" << headerName << ": " << headerValue << std::endl;
    }

	if (request.headers.find("Host") == request.headers.end()) {
		response.setStatusCode("400");
		return false;
	}
	return true;
}

std::string HttpRequest::getTimestamp() const {
	return timestamp;
}

void HttpRequest::generateTimestamp() {
	// Get the current time
    time_t currentTime;
    time(&currentTime);

    // Convert the current time to a string representation
    strftime(this->timestamp, sizeof(this->timestamp), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
}

bool HttpRequest::isRequestValid(char data[], HttpResponse& response) {
	std::string dataString(data);
	std::istringstream requestStream(dataString);
	std::string line;
	std::size_t pos;

	// Parse the request line
	std::getline(requestStream, line);
	std::istringstream requestLineStream(line);
	std::string method;
	std::string resource;
	std::string httpVersion;
	requestLineStream >> method >> resource >> httpVersion;

	// Print first line
	std::cout << "request line: " << line << std::endl;

	if (method.empty() || resource.empty() || httpVersion.empty()) {
		response.setStatusCode("400");
		return false;
	}

	std::cout << "method: " << method << std::endl;
	// Check if the request line is valid
	if (method != "GET" && method != "POST" && method != "DELETE") {
		std::cout << "method not valid" << std::endl;
		response.setStatusCode("405");
		return false;
	}

	if (httpVersion != "HTTP/1.1") {
		response.setStatusCode("505");
		return false;
	}

	// Check if the resource is valid
	if (resource.empty() || resource[0] != '/') {
		response.setStatusCode("400");
		return false;
	}

	// Check if the resource contains a double dot
	pos = resource.find("..");
	if (pos != std::string::npos) {
		response.setStatusCode("400");
		return false;
	}

	// Check if the resource contains a double slash
	pos = resource.find("//");
	if (pos != std::string::npos) {
		response.setStatusCode("400");
		return false;
	}

	return true;
}

/* int HttpRequest::getContentLength() const {
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
	if (it == headers.end()) {
		return -1;
	}
	return std::stoi(it->second);
} */
