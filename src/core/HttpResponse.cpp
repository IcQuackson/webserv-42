
#include "core/HttpResponse.hpp"

HttpResponse::HttpResponse() {
	this->statusCode = "";
	this->statusMessage = "";
	this->headers = std::map<std::string, std::string>();
	this->body = "";
}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &httpResponse) {
	httpVersion = httpResponse.httpVersion;
	statusCode = httpResponse.statusCode;
	statusMessage = httpResponse.statusMessage;
	headers = httpResponse.headers;
	body = httpResponse.body;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &httpResponse) {
	if (this != &httpResponse) {
		httpVersion = httpResponse.httpVersion;
		statusCode = httpResponse.statusCode;
		statusMessage = httpResponse.statusMessage;
		headers = httpResponse.headers;
		body = httpResponse.body;
	}
	return *this;
}

std::string HttpResponse::getHttpVersion() const {
	return httpVersion;
}

void HttpResponse::setHttpVersion(const std::string newHttpVersion) {
	httpVersion = newHttpVersion;
}

std::string HttpResponse::getStatusCode() const {
	return statusCode;
}

void HttpResponse::setStatusCode(const std::string newStatusCode) {
	statusCode = newStatusCode;
	statusMessage = HttpStatusCode::getHttpStatusCode(newStatusCode);
}

std::string HttpResponse::getStatusMessage() const {
	return statusMessage;
}

void HttpResponse::setStatusMessage(const std::string newStatusMessage) {
	statusMessage = newStatusMessage;
}

std::map<std::string, std::string> HttpResponse::getHeaders() const {
	return headers;
}

void HttpResponse::setHeaders(const std::map<std::string, std::string> newHeaders) {
	headers = newHeaders;
}

std::string HttpResponse::getBody() const {
	return body;
}

void HttpResponse::setBody(const std::string newBody) {
	body = newBody;
}

void HttpResponse::addHeader(const std::string header, const std::string value) {
	headers.insert(std::pair<std::string, std::string>(header, value));
}

std::string HttpResponse::toString() const {
	std::string response = "HTTP/1.1 " + statusCode + " " + statusMessage + "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += body + "\r\n";
	return response;
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& response) {
	os << response.toString();
	return os;
}
