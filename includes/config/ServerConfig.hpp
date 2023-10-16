#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class ServerConfig {

private:
	int nbr;
	std::string configFilePath;
	std::string host;
	std::vector<int> ports;
	std::vector<std::string> server_names;
	std::string root;
	int port;

	std::vector<Location*> locations;

public:
	ServerConfig();
	ServerConfig(const std::string& configFilePath, const std::string& host, int port);
	~ServerConfig();
	ServerConfig(ServerConfig const &serverConfig);
	ServerConfig &operator=(ServerConfig const &serverConfig);

	// getters and setters
	std::string getConfigFilePath();
	std::string getHost();
	std::vector<int> getPorts();
	std::vector<std::string> getServer_names();
	std::string getRoot();
	void addPort(int port);
	void addServer_name(std::string server_name);
	int getPort();
	int getNbr();
	std::vector<Location*> getLocations();
	void addLocation(Location* location);
	//void setConfigFilePath(const std::string& configFilePath);
	//void setHost(const std::string& host);
	void setNbr(int nbr);
	void setPorts(int ports);
	void setRoot(std::string root);
};