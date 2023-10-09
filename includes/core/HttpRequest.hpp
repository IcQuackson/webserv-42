
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>

class HttpRequest {

private:
	std::string method;
	std::string resource;
	std::string httpVersion;
	std::string host;
	std::map<std::string, std::string> args;
	std::map<std::string, std::string> headers;
	std::string body;

public:
	HttpRequest();
	~HttpRequest();
	HttpRequest(const HttpRequest &httpRequest);
	HttpRequest &operator=(const HttpRequest &httpRequest);
	
	std::string getMethod() const;
	std::string getResource() const;
	std::string getHttpVersion() const;
	std::string getHost() const;
	std::map<std::string, std::string> getArgs() const;
	std::map<std::string, std::string> getHeaders() const;
	std::string getBody() const;

	void setMethod(const std::string& method);
	void setResource(const std::string& resource);
	void setHttpVersion(const std::string& httpVersion);
	void setHost(const std::string& host);
	void setArgs(const std::map<std::string, std::string> args);
	void setHeaders(const std::map<std::string, std::string> & headers);
	bool readArgs(const std::string& resource);
	bool readHeaders(std::istringstream& requestStream, HttpRequest& request);
	void setBody(const std::string& body);
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);
