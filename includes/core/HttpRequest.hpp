
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "core/HttpResponse.hpp"
#include "core/HttpStatusCode.hpp"

class HttpRequest {

private:
	std::string method;
	std::string route;
	std::string resource;
	std::string httpVersion;
	std::string host;
	std::map<std::string, std::string> args;
	std::map<std::string, std::string> headers;
	std::string body;
	char timestamp[80];

public:
	HttpRequest();
	~HttpRequest();
	HttpRequest(const HttpRequest &httpRequest);
	HttpRequest &operator=(const HttpRequest &httpRequest);
	
	std::string getMethod() const;
	std::string getRoute() const;
	std::string getResource() const;
	std::string getHttpVersion() const;
	std::string getHost() const;
	std::map<std::string, std::string> getArgs() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	std::string getTimestamp() const;
	int getContentLength() const;

	void setMethod(const std::string& method);
	void setRoute(const std::string& route);
	void setResource(const std::string& resource);
	void setHttpVersion(const std::string& httpVersion);
	void setHost(const std::string& host);
	void setArgs(const std::map<std::string, std::string> args);
	void setHeaders(const std::map<std::string, std::string> & headers);
	bool readArgs(const std::string& resource);
	bool readHeaders(std::istringstream& requestStream, HttpRequest& request, HttpResponse& response);
	void setBody(const std::string& body);
	void setContentLength(int contentLength);
	void generateTimestamp();
	static bool isRequestValid(char data[], HttpResponse& response);
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);
