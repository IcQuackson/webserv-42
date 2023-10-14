#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "ServerConfig.hpp"

class ConfigParser {

private:

char *argv;
std::vector<ServerConfig*> serverConfigVector;
int server_in;

public:
	ConfigParser();
	ConfigParser(char *argv);
	~ConfigParser();
	ConfigParser(ConfigParser const &configParser);

	void proccess_input (void);

	//Parsing Aux
	bool checkServer(std::string token, std::stringstream& ss);

};