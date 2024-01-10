
#include "core/RouteHandler.hpp"
#include "core/HttpStatusCode.hpp"

RouteHandler::RouteHandler() {
}

RouteHandler::RouteHandler(const ServerConfig serverConfig, const Location location) : serverConfig(serverConfig),location(location) {
}

RouteHandler::~RouteHandler() {
}

RouteHandler &RouteHandler::operator=(RouteHandler const &routeHandler) {
	this->serverConfig = routeHandler.serverConfig;
	this->location = routeHandler.location;
	return *this;
}

Location RouteHandler::getLocation() const {
	return this->location;
}

void RouteHandler::setLocation(const Location& location) {
	this->location = location;
}

ServerConfig RouteHandler::getServerConfig() const {
	return this->serverConfig;
}

void RouteHandler::setServerConfig(const ServerConfig& serverConfig) {
	this->serverConfig = serverConfig;
}


void RouteHandler::handleRequest(HttpRequest& request, HttpResponse& response) {
	std::vector<std::string> methods = this->location.getMethods();
	std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), request.getMethod());
	
	//print methods
	
	//std::cout << "handleRequest: " << this->location.getMethods() << std::endl;

	std::cout << "Methods:";
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it) {
		std::cout << ' ' << *it;
	}
	std::cout << '\n';

	// Check if method is allowed
	if (it == location.getMethods().end()) {
		response.setStatusCode("405");
		return;
	}
	if (request.getMethod() == "GET") {
		this->handleGet(request, response);
	} else if (request.getMethod() == "POST") {
		this->handlePost(request, response);
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
	std::string index = location.getIndex();
	std::string path = root + resource;
	std::string indexPath = root + index;

	//std::cout << "HANDLE GET LOCATION: " << location << std::endl;
	std::cout << path << std::endl;

	if (!resourceExists(path)) {
		std::cerr << "Resource does not exist: " << path << std::endl;
		response.setStatusCode("404");
		return;
	}

	if (isDirectory(path)) {
		std::cout << "Directory" << std::endl;
		// Check if directory listing is enabled
		if (!location.getDirectoryListing()) {
			std::cerr << "Directory listing is disabled" << std::endl;
			response.setStatusCode("403");
			response.setBody("Directory listing is disabled");
			return;
		}
		// Check if the index file exists
		if (!index.empty() && resourceExists(indexPath) && !isDirectory(indexPath)) {
			std::cout << "Index file exists" << std::endl;
			std::cout << "Index file: " << indexPath << std::endl;
			path = indexPath;
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
		handleRegularFile(path, request, response);
	}
}

void RouteHandler::handleRegularFile(const std::string filePath, HttpRequest& request, HttpResponse& response) {
	(void) request;

	std::ifstream fileStream(filePath.c_str(), std::ios::binary);

    if (!fileStream) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
		response.setStatusCode("500");
		response.setBody("Failed to open file");
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
	std::string index = location.getIndex();
	std::string path = root + resource;
	std::string indexPath = root + index;
	std::string filename;
	//bool isFileUpload = false;
	

	std::cout << "POST METHOD(need to be done):  " << request.getBody() << std::endl;

	std::string upload_abs_path = location.getRoot() + location.getUploadPath();
	std::cout << upload_abs_path << std::endl;

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
			return ;
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
					response.setBody("Failed to open file");
					return ;
				}
    			ofile << request.getBody();
    			ofile.close();

				std::cout << "Data appended to the file." << std::endl;
			} else {
				std::cerr << "Error opening the file for appending." << std::endl;
			}
		}
	}
	// TODO Acabar de implementar o POST
/*
	TODO clientBodySize isClientBdyValid()

	if (!location.getAcceptUploads() && isFileUpload)
	{
		std::cerr << "Uploads are not allowed" << std::endl;
		response.setStatusCode("403");
		response.setBody("Uploads are not allowed");
		return ;
	}

	if (location.getAcceptUploads() && !isFileUpload)
	{
		std::cerr << "Uploads are allowed" << std::endl;
		response.setStatusCode("200");
		response.setBody("Uploads are allowed");
		return ;
	} */
		
		/* std::ofstream fileStream(upload_abs_path, std::ios::app);

		// Check if the file is open
		if (fileStream.is_open()) {
			// Write to the file
			fileStream << "This line will be appended to the file.\n";

			// Close the file
			fileStream.close();

			std::cout << "Data appended to the file." << std::endl;
		} else {
			std::cerr << "Error opening the file for appending." << std::endl;
		} */


	//std::cout << "HANDLE GET LOCATION: " << location << std::endl;
	/* std::cout << path << std::endl;

	if (!resourceExists(path)) {
		std::cerr << "Resource does not exist: " << path << std::endl;
		response.setStatusCode("404");
		return;
	}

	if (isDirectory(path)) {
		std::cout << "Directory" << std::endl;
		// Check if directory listing is enabled
		if (!location.getDirectoryListing()) {
			std::cerr << "Directory listing is disabled" << std::endl;
			response.setStatusCode("403");
			response.setBody("Directory listing is disabled");
			return;
		}
		// Check if the index file exists
		if (!index.empty() && resourceExists(indexPath) && !isDirectory(indexPath)) {
			std::cout << "Index file exists" << std::endl;
			std::cout << "Index file: " << indexPath << std::endl;
			path = indexPath;
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
		handleRegularFile(path, request, response);
	} */
}
