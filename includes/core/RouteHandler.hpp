
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include "config/Location.hpp"
#include "core/HttpRequest.hpp"
#include "core/HttpResponse.hpp"

class RouteHandler {

private:
	Location location;

public:

	RouteHandler();

	RouteHandler(const Location& location);

	~RouteHandler();

	RouteHandler &operator=(RouteHandler const &routeHandler);

	Location getLocation() const;

	void setLocation(const Location& location);

	void handleRequest(HttpRequest& request, HttpResponse& response);

	bool resourceExists(const std::string& path);

	bool isDirectory(const std::string& path);

	void handleGet(HttpRequest& request, HttpResponse& response);

	void handlePost(HttpRequest& request, HttpResponse& response);

	void handleDelete(HttpRequest& request, HttpResponse& response);

	void handleDirectoryListing(HttpRequest& request, HttpResponse& response);

	void handleRegularFile(const std::string filePath, HttpRequest& request, HttpResponse& response);

	void handleCgi(const std::string filePath, HttpRequest& request, HttpResponse& response);

	long getFileSize(const std::string& filePath);

	std::string getContentType(const std::string& filePath);

	template <typename T>
	static std::string numberToString(T number);
};