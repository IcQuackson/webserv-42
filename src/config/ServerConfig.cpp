#include "config/ServerConfig.hpp"

ServerConfig::ServerConfig() {
}

ServerConfig::ServerConfig(const std::string& configFilePath, const std::string& host, int port) {
	this->configFilePath = configFilePath;
	this->host = host;
	this->port = port;
}

ServerConfig::~ServerConfig() {
}

ServerConfig::ServerConfig(ServerConfig const &serverConfig) {
	*this = serverConfig;
}

ServerConfig &ServerConfig::operator=(ServerConfig const &serverConfig) {
	if (this != &serverConfig) {
		this->configFilePath = serverConfig.configFilePath;
		this->host = serverConfig.host;
		this->port = serverConfig.port;
	}
	return *this;
}

std::string ServerConfig::getConfigFilePath() {
	return this->configFilePath;
}

std::string ServerConfig::getHost() {
	return this->host;
}

std::vector<int> ServerConfig::getPorts() {
	return this->ports;
}

std::vector<std::string> ServerConfig::getServer_names(){
	return this->server_names;
}


int ServerConfig::getPort() {
	return this->port;
}

int ServerConfig::getNbr() {
	return this->nbr;
}

std::vector<Location*> ServerConfig::getLocations() {
	return this->locations;
}

std::string ServerConfig::getRoot() {
	return this->root;
}

std::vector<int> ServerConfig::getError_codes() {
	return this->error_codes;
}

std::vector<std::string> ServerConfig::getError_pages() {
	return this->error_pages;
}


void ServerConfig::addLocation(Location* location) {
	this->locations.push_back(location);
}

void ServerConfig::addPort(int port)
{
	this->ports.push_back(port);
}

void ServerConfig::addServer_name(std::string server_name)
{
	this->server_names.push_back(server_name);
}

void ServerConfig::addError_code(int error_code) {
	this->error_codes.push_back(error_code);
}

void ServerConfig::addError_page(std::string error_page) {
	this->error_pages.push_back(error_page);
}


void ServerConfig::setPorts(int port)
{
	this->port = port;
}

void ServerConfig::setNbr(int nbr)
{
	this->nbr = nbr;
}

void ServerConfig::setRoot(std::string root)
{
	this->root = root;
}
