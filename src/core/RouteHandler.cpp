
#include "core/RouteHandler.hpp"
#include "core/HttpStatusCode.hpp"

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

void RouteHandler::handleRequest(HttpRequest& request, HttpResponse& response) {
	std::vector<std::string> methods = this->location.getMethods();
	std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), request.getMethod());
	
	std::cout << "handleRequest " << this->location << std::endl;

	// Check if method is allowed
	if (it == location.getMethods().end()) {
		HttpStatusCode::setCurrentStatusCode("405");
		response.setStatusCode("405");
		return;
	}
	if (request.getMethod() == "GET") {
		this->handleGet(request, response);
	} else if (request.getMethod() == "POST") {
		//this->handlePost(request, response);
	} else if (request.getMethod() == "DELETE") {
		//this->handleDelete(request, response);
	}
}

bool RouteHandler::resourceExists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool RouteHandler::isDirectory(const std::string& path) {
	struct stat s;

	if (stat(path.c_str(), &s) == 0) {
		if (s.st_mode & S_IFDIR) {
			return true;
		}
	}
	return false;
}



void RouteHandler::handleGet(HttpRequest& request, HttpResponse& response) {
	Location location = getLocation();
	std::string resource = request.getResource();
	std::string root = location.getRoot();
	std::string index = location.getIndex();
	std::string path = root + resource;
	std::string indexPath = root + index;

	std::cout << "HANDLE GET LOCATION: " << location << std::endl;

	if (!resourceExists(path)) {
		std::cerr << "Resource does not exist: " << path << std::endl;
		HttpStatusCode::setCurrentStatusCode("404");
		response.setStatusCode("404");
		return;
	}

	if (isDirectory(path)) {
		// Check if directory listing is enabled
		if (!location.getDirectoryListing()) {
			std::cerr << "Directory listing is disabled" << std::endl;
			HttpStatusCode::setCurrentStatusCode("403");
			response.setStatusCode("403");
			return;
		}
		// Check if the index file exists
		if (resourceExists(indexPath)) {
			path = indexPath;
		}
		else {
			//handleDirectoryListing(request, response);
			return;
		}
	}

	// Check if the resource is a CGI script
	if (path.substr(path.find_last_of(".") + 1) == location.getCgiExtension()) {
		std::cerr << "CGI script" << std::endl;
		//handleCgi(request, response);
	}
	else {
		std::cerr << "Regular file" << std::endl;
		//handleRegularFile(path, request, response);
	}
}

void RouteHandler::handleRegularFile(const std::string filePath, HttpRequest& request, HttpResponse& response) {
	(void) request;

	std::ifstream fileStream(filePath.c_str(), std::ios::binary);

    if (!fileStream) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        HttpStatusCode::setCurrentStatusCode("500");
		response.setStatusCode("500");
        return;
    }

	// Determine the content type based on file extension
    std::string contentType = getContentType(filePath);

    response.addHeader("Content-Type", contentType);
    response.addHeader("Content-Length", numberToString(getFileSize(filePath)));

	// Read the file into the response body
	std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	response.setBody(fileContent);

	fileStream.close();
}

std::string RouteHandler::getContentType(const std::string& filePath) {
	std::string contentType;
	std::string extension = filePath.substr(filePath.find_last_of(".") + 1);

	if (extension == "html" || extension == "htm") {
		contentType = "text/html";
	}
	else if (extension == "css") {
		contentType = "text/css";
	}
	else if (extension == "js") {
		contentType = "text/javascript";
	}
	else if (extension == "jpg" || extension == "jpeg") {
		contentType = "image/jpeg";
	}
	else if (extension == "png") {
		contentType = "image/png";
	}
	else if (extension == "gif") {
		contentType = "image/gif";
	}
	else if (extension == "ico") {
		contentType = "image/x-icon";
	}
	else if (extension == "txt") {
		contentType = "text/plain";
	}
	else if (extension == "pdf") {
		contentType = "application/pdf";
	}
	else if (extension == "json") {
		contentType = "application/json";
	}
	else {
		contentType = "application/octet-stream";
	}

	return contentType;
}

long RouteHandler::getFileSize(const std::string& filePath) {
	struct stat stat_buf;
	int rc = stat(filePath.c_str(), &stat_buf);

	return rc == 0 ? stat_buf.st_size : -1;
}

template <typename T>
std::string RouteHandler::numberToString(T number) {
	std::ostringstream oss;
	oss << number;
	return oss.str();
}

