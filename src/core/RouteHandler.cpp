
#include "core/RouteHandler.hpp"

RouteHandler::RouteHandler() {
}

RouteHandler::RouteHandler(const Location& location) : location(location) {
}

RouteHandler::~RouteHandler() {
}

RouteHandler &RouteHandler::operator=(RouteHandler const &routeHandler) {
	this->location = routeHandler.location;
	return *this;
}

Location RouteHandler::getLocation() const {
	return this->location;
}

void RouteHandler::setLocation(const Location& location) {
	this->location = location;
}
