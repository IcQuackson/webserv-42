
#pragma once

#include <string>
#include <vector>
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

	void handleGet(HttpRequest& request, HttpResponse& response);

	void handlePost(HttpRequest& request, HttpResponse& response);

	void handleDelete(HttpRequest& request, HttpResponse& response);
};