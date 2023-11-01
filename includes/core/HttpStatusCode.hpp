#pragma once

#include <string>
#include <map>
#include <utility>
#include <iostream>


class HttpStatusCode {

public:
	static std::string currentStatusCode;
	static std::map<std::string, std::string> statusCodes;

	static void initStatusCodes();
	static std::pair<std::string, std::string> getCurrentStatusCode();
	static void setCurrentStatusCode(const std::string statusCode);
	static std::string getHttpStatusCode(const std::string& statusCode);

private:
	HttpStatusCode();
};
