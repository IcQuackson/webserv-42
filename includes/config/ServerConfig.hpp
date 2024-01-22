#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class ServerConfig {

private:
	int nbr;
	std::string configFilePath;
	int port;
	std::string host;
	std::vector<std::string> server_names;
	std::vector<int> error_codes;
	std::string error_page_path;
	std::string client_max_body_size;
	std::vector<Location*> locations;

public:
	ServerConfig();
	ServerConfig(const std::string& configFilePath, const std::string& host, int port);
	~ServerConfig();
	ServerConfig(ServerConfig const &serverConfig);
	ServerConfig &operator=(ServerConfig const &serverConfig);

	std::string getConfigFilePath();
	int getPort();
	std::string getHost();
	std::vector<std::string> getServer_names();
	std::vector<int> getError_codes();
	std::string getErrorPage();
	int getNbr();
	std::string getClient_max_body_size();
	std::vector<Location*> getLocations();
	void addHost(std::string host);
	void addPort(int port);
	void addServer_name(std::string server_name);
	void addLocation(Location* location);
	void addError_code(int error_code);
	void setErrorPage(std::string error_page_path);
	void setNbr(int nbr);
	void setPort(int port);
	void setHost(std::string host);
	void setClient_max_body_size(std::string client_max_body_size);
	static void setDefaultServer(ServerConfig *serverConfig);
	void delete_mem();
};