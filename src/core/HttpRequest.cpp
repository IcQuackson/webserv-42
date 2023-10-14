#include "core/HttpRequest.hpp"
#include "core/HttpStatusCode.hpp"

HttpRequest::HttpRequest() {
	args = std::map<std::string, std::string>();
	headers = std::map<std::string, std::string>();
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &httpRequest) {
    method = httpRequest.method;
    resource = httpRequest.resource;
    httpVersion = httpRequest.httpVersion;
    host = httpRequest.host;
    args = httpRequest.args;
	headers = httpRequest.headers;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &httpRequest) {
    if (this != &httpRequest) {
        method = httpRequest.method;
        resource = httpRequest.resource;
        httpVersion = httpRequest.httpVersion;
        host = httpRequest.host;
        args = httpRequest.args;
		headers = httpRequest.headers;
    }
    return *this;
}

std::string HttpRequest::getMethod() const {
	return method;
}

void HttpRequest::setMethod(const std::string& newMethod) {
	method = newMethod;
}

std::string HttpRequest::getResource() const {
	return resource;
}

void HttpRequest::setResource(const std::string& newResource) {
	resource = newResource;
}

std::string HttpRequest::getHttpVersion() const {
	return httpVersion;
}

void HttpRequest::setHttpVersion(const std::string& newHttpVersion) {
	httpVersion = newHttpVersion;
}

std::string HttpRequest::getHost() const {
	return host;
}

void HttpRequest::setHost(const std::string& newHost) {
	host = newHost;
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
	return body;
}

void HttpRequest::setBody(const std::string& newBody) {
	body = newBody;
}

bool HttpRequest::readArgs(const std::string& resource) {
	std::string argString;

	(void) args;

	// Find the position of the first question mark
	std::size_t pos = resource.find("?");
	if (pos == std::string::npos) {
		return false;
	}

	// Get the argument string
	argString = resource.substr(pos + 1);

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

bool HttpRequest::readHeaders(std::istringstream& requestStream, HttpRequest& request) {
    std::string line;

    while (std::getline(requestStream, line) && line != "\r") {
        std::istringstream headerLineStream(line);
        std::string headerName;
        std::getline(headerLineStream, headerName, ':');
        std::string headerValue;
        std::getline(headerLineStream, headerValue);
        request.headers[headerName] = headerValue;
		std::cout << headerName << ": " << headerValue << std::endl;
    }

	if (request.headers.find("Host") == request.headers.end()) {
		HttpStatusCode::setCurrentStatusCode("400");
		return false;
	}
	return true;
}

bool HttpRequest::isRequestValid(char data[]) {
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

	if (method.empty() || resource.empty() || httpVersion.empty()) {
		HttpStatusCode::setCurrentStatusCode("400");
		return false;
	}
	// Check if the request line is valid
	if (method != "GET" && method != "POST" && method != "DELETE") {
		std::cout << "Invalid method: " << method << std::endl;
		HttpStatusCode::setCurrentStatusCode("405");
		return false;
	}

	if (httpVersion != "HTTP/1.1") {
		HttpStatusCode::setCurrentStatusCode("505");
		return false;
	}

	// Check if the resource is valid
	if (resource.empty() || resource[0] != '/') {
		HttpStatusCode::setCurrentStatusCode("400");
		return false;
	}

	// Check if the resource contains a double dot
	pos = resource.find("..");
	if (pos != std::string::npos) {
		HttpStatusCode::setCurrentStatusCode("400");
		return false;
	}

	// Check if the resource contains a double slash
	pos = resource.find("//");
	if (pos != std::string::npos) {
		HttpStatusCode::setCurrentStatusCode("400");
		return false;
	}
	

	return true;
}
