#include "core/RouteHandler.hpp"
#include "core/HttpStatusCode.hpp"
#include "cgi/CgiHandler.hpp"

RouteHandler::RouteHandler() {
}

RouteHandler::RouteHandler(const Location location) : location(location) {
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

	// Check if method is allowed
	if (it == methods.end()) {
		std::cerr << "Method not allowed: " << request.getMethod() << std::endl;
		response.setStatusCode("405");
		return;
	}
	if (this->location.getClientBodySize() < (int)request.getBody().length()) {
		std::cerr << "Request body too large: " << request.getBody().length() << std::endl;
		response.setStatusCode("413");
		return;
	}
	if (request.getMethod() == "GET") {
		this->handleGet(request, response);
	}
	else if (request.getMethod() == "POST") {
		this->handlePost(request, response);
	}
	else if (request.getMethod() == "DELETE") {
		this->handleDelete(request, response);
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

void RouteHandler::generateDirectoryListing(const std::string& root, const std::string& path, std::ostringstream& response) {
	response << "Directory Listing:\n";

	DIR* dir;
	struct dirent* entry;

	if ((dir = opendir((root + path).c_str())) != NULL) {
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
				response << entry->d_name;
				if (entry->d_type == DT_DIR) {
					response << "/";
				}
				response << "\n";
			}
		}
		closedir(dir);
	}
}

void RouteHandler::handleDirectoryListing(HttpResponse& response, const std::string& root, const std::string& resource) {
	std::ostringstream responseStream;
	std::cout << "Directory listing" << std::endl;
	response.setStatusCode("200");

	generateDirectoryListing(root, resource, responseStream);

	// Set up response
	std::string contentType = "text/html";
	std::string directoryListing = responseStream.str();

	response.setBody(directoryListing);
	response.addHeader("Content-Type", contentType);
	response.addHeader("Content-Length", numberToString(directoryListing.length()));
}


void RouteHandler::handleGet(HttpRequest& request, HttpResponse& response) {
	Location location = getLocation();
	std::string resource = request.getResource();
	std::string root = location.getRoot();
	std::string route = request.getRoute();
	std::string index = location.getDefaultFile();
	std::string path = root + resource;
	std::string indexPath = root + index;
	
	if(route == "/cgi-bin")
	{
		if (!resource.empty() || resource == "/") {
			std::cerr << "Resource is forbidden: " << path << std::endl;
			response.setStatusCode("401");
			return;
		}
		CgiHandler cgi;
		cgi.exec_cgi_py(request, response, *this, 0);
		return;
	}
	if (!resourceExists(path)) {
		std::cerr << "Resource does not exist: " << path << std::endl;
		response.setStatusCode("404");
		return;
	}

	if (path != "./" && path[path.length() - 1] == '/') {
		path = path.substr(0, path.length() - 1);
	}
	
	if (isDirectory(path)) {
		// Check if the index file exists
		if (!index.empty() && path == root && resourceExists(indexPath) && !isDirectory(indexPath)) {
			handleRegularFile(indexPath, response);
			return;
		}
		// Check if directory listing is enabled
		if (!location.getDirectoryListing()) {
			std::cerr << "Directory listing is disabled" << std::endl;
			response.setStatusCode("403");
			return;
		}
		else {
			handleDirectoryListing(response, root, resource);
			return;
		}
	}
	// Check if the resource is a CGI script
	else if (path.substr(path.find_last_of(".") + 1) == location.getCgiExtension()) {
		std::cerr << "CGI script" << std::endl;
		//handleCgi(request, response);
	}
	else {
		std::cerr << "Regular file" << std::endl;
		handleRegularFile(path, response);
	}
}

void RouteHandler::handleRegularFile(const std::string filePath, HttpResponse& response) {

	std::ifstream fileStream(filePath.c_str(), std::ios::binary);

    if (!fileStream) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
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
	std::cout << "File Content: " << fileContent << std::endl;

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

bool RouteHandler::extract_filename(HttpRequest& request, std::string &filename)
{
	std::string inputString;
	std::string filenameKey = "filename=\"";
	
	
	if (request.getHeaders().find("Content-Disposition") != request.getHeaders().end()
		&& !request.getHeaders()["Content-Disposition"].empty()) {
		inputString = request.getHeaders().find("Content-Disposition")->second;
	}
	else
		return (0);
	size_t filenamePos = inputString.find(filenameKey);
	if (filenamePos != std::string::npos) 
	{
		// Extract the content after the filename key
		filenamePos += filenameKey.length();
		size_t endPos = inputString.find("\"", filenamePos);

		if (endPos != std::string::npos)
		{
			filename = inputString.substr(filenamePos, endPos - filenamePos);
			return (1);
		}
		else
			return (0);
	}
	else
		return (0);
}

void RouteHandler::handlePost(HttpRequest& request, HttpResponse& response) {
	Location location = getLocation();
	std::string resource = request.getResource();
	std::string root = location.getRoot();
	std::string route = request.getRoute();
	std::string index = location.getIndex();
	std::string path = root + resource;
	std::string indexPath = root + index;
	std::string filename;

	std::string upload_abs_path = location.getRoot() + location.getUploadPath();
	if(route == "/cgi-bin")
	{
		if (!resource.empty() || resource == "/") {
			std::cerr << "Resource is forbidden: " << path << std::endl;
			response.setStatusCode("401");
			return;
		}
		CgiHandler cgi;
		cgi.exec_cgi_py(request,response, *this, 1);
		return;
	}

	if(!location.getAcceptUploads())
	{
		std::cerr << "Uploads are not allowed" << std::endl;
		response.setStatusCode("403");
		return ;
	}

	if (!isDirectory(upload_abs_path)) {
		std::cerr << "Upload path does not exist: " << path << std::endl;
		response.setStatusCode("404");
		return;
	}
	else
	{
		if (!request.getBody().empty())
			filename = "default.txt";
		if (!extract_filename(request, filename))
		{
			if (!request.getBody().empty())
				filename = "default.txt";
			else
			{
				std::cerr << "Error: Filename key not found." << std::endl;
				response.setStatusCode("400");
				return ;
			}
		}
		if (filename.empty())
		{
			response.setStatusCode("400");
			return ;
		}
		else
		{
			std::ofstream ofile;

			ofile.open((upload_abs_path + "/" + filename).c_str());
			// Check if the file is open
			if (ofile.is_open()) 
			{
				if(!(ofile.is_open()))
				{
					std::cerr << "Failed to open file: " << (upload_abs_path + "/" + filename) << std::endl;
					response.setStatusCode("500");
					return ;
				}
    			ofile << request.getBody();
    			ofile.close();

				std::cout << "Upload sucessfull" << std::endl;
				response.setBody("Upload successful");
				response.setStatusCode("201");
			} 
			else
			{
				response.setStatusCode("500");
				std::cerr << "Error opening the file for appending." << std::endl;
			}
		}
	}	
}

bool deleteDirectory(const char* path) {
    DIR* dir = opendir(path);

    if (dir == NULL) {
        std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
        return false;
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string filePath = std::string(path) + "/" + entry->d_name;

            if (entry->d_type == DT_DIR) {
                if (!deleteDirectory(filePath.c_str()))
                    return false;
            } else {
                if (remove(filePath.c_str()) != 0) {
                    std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
                    return false;
                }
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0) {
        std::cerr << "Error deleting directory: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

void RouteHandler::handleDelete(HttpRequest& request, HttpResponse& response) {
	Location location = getLocation();
	std::string resource = request.getResource();
	std::string root = location.getRoot();
	std::string path = root + resource;

	if (!resourceExists(path)) {
		std::cerr << "Resource does not exist: " << path << std::endl;
		response.setStatusCode("404");
		return;
	}

	if (isDirectory(path)) {
		if (deleteDirectory(path.c_str())) {
        	std::cout << "Directory deleted successfully." << std::endl;
			response.setStatusCode("204");
    	} else {
        	std::cerr << "Error removing directory." << std::endl;
			response.setStatusCode("500");
    	}
	} else if (remove(path.c_str()) != 0) {
            std::cerr << "Error removing file: " << strerror(errno) << std::endl;
            response.setStatusCode("500");
	} else {
		std::cout << "File removed successfully." << std::endl;
        response.setStatusCode("204");
	}
}
