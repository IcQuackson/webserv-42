#include "config/ServerConfig.hpp"

ServerConfig::ServerConfig() {
}

ServerConfig::ServerConfig(const std::string& configFilePath, const std::string& host, int port) {
	this->configFilePath = configFilePath;
	this->port = port;
	this->host = host;
}

ServerConfig::~ServerConfig() {
}

ServerConfig::ServerConfig(ServerConfig const &serverConfig) {
	*this = serverConfig;
}

ServerConfig &ServerConfig::operator=(ServerConfig const &serverConfig) {
	if (this != &serverConfig) {
		this->nbr = serverConfig.nbr;
		this->configFilePath = serverConfig.configFilePath;
		this->host = serverConfig.host;
		this->port = serverConfig.port;
		this->server_names = serverConfig.server_names;
		this->error_codes = serverConfig.error_codes;
		this->error_pages = serverConfig.error_pages;
		this->client_max_body_size = serverConfig.client_max_body_size;
		this->locations = serverConfig.locations;
	}
	return *this;
}

std::string ServerConfig::getConfigFilePath() {
	return this->configFilePath;
}

std::string ServerConfig::getHost() {
	return this->host;
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

std::vector<int> ServerConfig::getError_codes() {
	return this->error_codes;
}

std::vector<std::string> ServerConfig::getError_pages() {
	return this->error_pages;
}

std::string ServerConfig::getClient_max_body_size() {
	return this->client_max_body_size;
}


void ServerConfig::addLocation(Location* location) {
	this->locations.push_back(location);
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

void ServerConfig::setHost(std::string host) {
	this->host = host;
}

void ServerConfig::setPort(int port)
{
	this->port = port;
}

void ServerConfig::setNbr(int nbr)
{
	this->nbr = nbr;
}

void ServerConfig::setClient_max_body_size(std::string client_max_body_size)
{
	this->client_max_body_size = client_max_body_size;
}

void ServerConfig::setDefaultServer(ServerConfig *serverConfig) {
	Location *location;

	std::string host("localhost");
	serverConfig->setHost(host);
	serverConfig->setPort(8080);
	location = new Location();
	location->addMethod("GET");
	location->addMethod("POST");
	location->addMethod("DELETE");
	location->setAcceptUploads(true);
	location->setDirectoryListing(true);
	location->setRoot("./YoupiBanane"); // É Suposto haver root no server cofig e na location? acho que so na location
	location->setUploadPath("/upload");
	location->setPath("/resource");
	location->setClientBodySize(10);
	serverConfig->addLocation(location);
}

