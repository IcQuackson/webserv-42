#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>

class HttpResponse {

private:
	std::string httpVersion;
	std::string statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;

public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse &httpResponse);
	HttpResponse &operator=(const HttpResponse &httpResponse);

	std::string getHttpVersion() const;
	std::string getStatusCode() const;
	std::string getStatusMessage() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;

	void setHttpVersion(const std::string httpVersion);
	void setStatusCode(const std::string statusCode);
	void setStatusMessage(const std::string statusMessage);
	void addHeader(const std::string header, const std::string value);
	void setHeaders(const std::map<std::string, std::string> headers);
	void setBody(const std::string body);
	std::string toString() const;
};

std::ostream& operator<<(std::ostream& os, const HttpResponse& response);