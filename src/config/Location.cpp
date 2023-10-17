
#include "../includes/config/Location.hpp"

Location::Location() {
	this->methods = std::vector<std::string>();
	this->indexFile = "";
	this->redirection = "";
	this->root = "";
	this->directoryListing = false;
	this->defaultFile = "";
	this->cgiExtension = "";
	this->acceptUploads = false;
	this->uploadEndpoint = "";
	this->uploadPath = "";
}

Location::Location(const std::string& path) : path(path) {
	this->methods = std::vector<std::string>();
	this->indexFile = "";
	this->redirection = "";
	this->root = "";
	this->directoryListing = false;
	this->defaultFile = "";
	this->cgiExtension = "";
	this->acceptUploads = false;
	this->uploadEndpoint = "";
	this->uploadPath = "";
}

Location::~Location() {
}

std::string Location::getPath() const {
	return this->path;
}

std::string Location::getIndex() const {
	return this->indexFile;
}

std::vector<std::string> Location::getMethods() const {
	return this->methods;
}

std::string Location::getRedirection() const {
	return this->redirection;
}

std::string Location::getRoot() const {
	return this->root;
}

bool Location::getDirectoryListing() const {
	return this->directoryListing;
}

std::string Location::getDefaultFile() const {
	return this->defaultFile;
}

std::string Location::getCgiExtension() const {
	return this->cgiExtension;
}

bool Location::getAcceptUploads() const {
	return this->acceptUploads;
}

std::string Location::getUploadEndpoint() const {
	return this->uploadEndpoint;
}

std::string Location::getUploadPath() const {
	return this->uploadPath;
}

void Location::addMethod(std::string method) {
	this->methods.push_back(method);
}

void Location::setPath(const std::string& path) {
	this->path = path;
}

void Location::setIndex(const std::string& indexFile) {
	this->indexFile = indexFile;
}

void Location::setMethods(const std::vector<std::string>& methods) {
	this->methods = methods;
}

void Location::setRedirection(const std::string& redirection) {
	this->redirection = redirection;
}

void Location::setRoot(const std::string& root) {
	this->root = root;
}

void Location::setDirectoryListing(bool directoryListing) {
	this->directoryListing = directoryListing;
}

void Location::setDefaultFile(const std::string& defaultFile) {
	this->defaultFile = defaultFile;
}

void Location::setCgiExtension(const std::string& cgiExtension) {
	this->cgiExtension = cgiExtension;
}

void Location::setAcceptUploads(bool acceptUploads) {
	this->acceptUploads = acceptUploads;
}

void Location::setUploadEndpoint(const std::string& uploadEndpoint) {
	this->uploadEndpoint = uploadEndpoint;
}

void Location::setUploadPath(const std::string& uploadPath) {
	this->uploadPath = uploadPath;
}
