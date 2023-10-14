
#include "core/HttpStatusCode.hpp"

std::string HttpStatusCode::currentStatusCode = "200 OK";
std::map<std::string, std::string> HttpStatusCode::statusCodes;

void HttpStatusCode::initStatusCodes() {
	statusCodes["100"] = "Continue";
	statusCodes["101"] = "Switching Protocols";
	statusCodes["102"] = "Processing";
	statusCodes["103"] = "Early Hints";
	statusCodes["200"] = "OK";
	statusCodes["201"] = "Created";
	statusCodes["202"] = "Accepted";
	statusCodes["203"] = "Non-Authoritative Information";
	statusCodes["204"] = "No Content";
	statusCodes["205"] = "Reset Content";
	statusCodes["206"] = "Partial Content";
	statusCodes["207"] = "Multi-Status";
	statusCodes["208"] = "Already Reported";
	statusCodes["226"] = "IM Used";
	statusCodes["300"] = "Multiple Choices";
	statusCodes["301"] = "Moved Permanently";
	statusCodes["302"] = "Found";
	statusCodes["303"] = "See Other";
	statusCodes["304"] = "Not Modified";
	statusCodes["305"] = "Use Proxy";
	statusCodes["306"] = "Switch Proxy";
	statusCodes["307"] = "Temporary Redirect";
	statusCodes["308"] = "Permanent Redirect";
	statusCodes["400"] = "Bad Request";
	statusCodes["401"] = "Unauthorized";
	statusCodes["402"] = "Payment Required";
	statusCodes["403"] = "Forbidden";
	statusCodes["404"] = "Not Found";
	statusCodes["405"] = "Method Not Allowed";
	statusCodes["406"] = "Not Acceptable";
	statusCodes["407"] = "Proxy Authentication Required";
	statusCodes["408"] = "Request Timeout";
	statusCodes["409"] = "Conflict";
	statusCodes["410"] = "Gone";
	statusCodes["411"] = "Length Required";
	statusCodes["412"] = "Precondition Failed";
	statusCodes["413"] = "Payload Too Large";
	statusCodes["414"] = "URI Too Long";
	statusCodes["415"] = "Unsupported Media Type";
	statusCodes["416"] = "Range Not Satisfiable";
	statusCodes["417"] = "Expectation Failed";
	statusCodes["418"] = "I'm a teapot";
	statusCodes["421"] = "Misdirected Request";
	statusCodes["422"] = "Unprocessable Entity";
	statusCodes["423"] = "Locked";
	statusCodes["424"] = "Failed Dependency";
	statusCodes["425"] = "Too Early";
	statusCodes["426"] = "Upgrade Required";
	statusCodes["428"] = "Precondition Required";
	statusCodes["429"] = "Too Many Requests";
	statusCodes["431"] = "Request Header Fields Too Large";
	statusCodes["451"] = "Unavailable For Legal Reasons";
	statusCodes["500"] = "Internal Server Error";
	statusCodes["501"] = "Not Implemented";
	statusCodes["502"] = "Bad Gateway";
	statusCodes["503"] = "Service Unavailable";
	statusCodes["504"] = "Gateway Timeout";
	statusCodes["505"] = "HTTP Version Not Supported";
}

std::pair<std::string, std::string> HttpStatusCode::getCurrentStatusCode() {
	return std::make_pair(currentStatusCode, statusCodes[currentStatusCode]);
}

void HttpStatusCode::setCurrentStatusCode(const std::string statusCode) {
	if (statusCodes.find(statusCode) == statusCodes.end()) {
		currentStatusCode = "500";
		return;
	}
	currentStatusCode = statusCode;
}

std::string HttpStatusCode::getHttpStatusCode(const std::string& statusCode) {
	return statusCodes[statusCode];
}
