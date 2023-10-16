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

const char *argv;
std::vector<ServerConfig*> serverConfigVector;
int server_in;

public:
	ConfigParser();
	ConfigParser(const char *argv);
	~ConfigParser();
	ConfigParser(ConfigParser const &configParser);

    std::vector<ServerConfig*> getServerConfigVector();

	void proccess_input (void);

	//Parsing Aux
	bool checkServer(std::string token, std::stringstream& ss);
    bool parse_listen(std::string &token, std::stringstream& ss);
    bool parse_server_name(std::string &token, std::stringstream& ss);
    bool parse_root(std::string &token, std::stringstream& ss);
    bool parse_location(std::string &token, std::stringstream& ss);




};