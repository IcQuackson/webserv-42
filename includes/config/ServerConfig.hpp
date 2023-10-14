#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class ServerConfig {

private:
	std::string configFilePath;
	std::string host;
	int port;

	std::vector<Location> locations;

public:
	ServerConfig();
	ServerConfig(const std::string& configFilePath, const std::string& host, int port);
	~ServerConfig();
	ServerConfig(ServerConfig const &serverConfig);
	ServerConfig &operator=(ServerConfig const &serverConfig);

	// getters and setters
	std::string getConfigFilePath();
	std::string getHost();
	int getPort();
	std::vector<Location> getLocations();
	void addLocation(Location location);
	//void setConfigFilePath(const std::string& configFilePath);
	//void setHost(const std::string& host);
	//void setPort(int port);
};