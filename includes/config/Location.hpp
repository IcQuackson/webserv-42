#include <string>
#include <iostream>
#include <vector>

#pragma once

class Location {

private:
    
	std::string path;                    // Path of the route

	std::string indexFile;               // Index file for a directory

	std::vector<std::string> methods;    // List of accepted HTTP methods

	std::string redirection;             // HTTP redirection URL

	std::string root;                    // Directory or file from where files should be searched
	
	bool directoryListing;               // Flag to turn on/off directory listing

	std::string defaultFile;             // Default file for a directory

	std::string cgiPath;                 // Path for CGI executable

	std::string cgiExtension;            // File extension for CGI execution

	bool acceptUploads;                  // Flag to accept uploaded files

	std::string uploadEndpoint;          // Endpoint for handling uploaded files

	std::string uploadPath;              // Directory where uploaded files should be saved

	int clientBodySize;		 	         // Max size of the body of a request

    std::string error_page;              // Error page path

public:

	Location();
    Location(const std::string& path);
	~Location();

    std::string getPath() const;

	std::string getIndex() const;

    std::vector<std::string> getMethods() const;

    std::string getRedirection() const;

    std::string getRoot() const;

    bool getDirectoryListing() const;

    std::string getDefaultFile() const;

    std::string getCgiPath() const;

    std::string getCgiExtension() const;

    bool getAcceptGet() const;

    bool getAcceptPost() const;

    bool getAcceptUploads() const;

    std::string getUploadEndpoint() const;

    std::string getUploadPath() const;

	int getClientBodySize() const;

    std::string getErrorPage() const;

    std::vector<int> getErrorCodes() const;

    void addMethod(std::string method);

    void setPath(const std::string& path);

	void setIndex(const std::string& indexFile);

    void setMethods(const std::vector<std::string>& methods);

    void setRedirection(const std::string& redirection);

    void setRoot(const std::string& root);

    void setDirectoryListing(bool directoryListing);

    void setDefaultFile(const std::string& defaultFile);

    void setCgiPath(const std::string& cgiPath);

    void setCgiExtension(const std::string& cgiExtension);

    void setAcceptGet(bool acceptGet);

    void setAcceptPost(bool acceptPost);

    void setAcceptUploads(bool acceptUploads);

    void setUploadEndpoint(const std::string& uploadEndpoint);

    void setUploadPath(const std::string& uploadPath);

	void setClientBodySize(int clientBodySize);

    void setErrorPage(const std::string& error_page);

};

std::ostream& operator<<(std::ostream& os, const Location& location);