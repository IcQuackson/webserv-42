#pragma once

#include <string>

class ServerConfig {

private:
	std::string configFilePath;
	std::string host;
	int port;

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
	//void setConfigFilePath(const std::string& configFilePath);
	//void setHost(const std::string& host);
	//void setPort(int port);
};