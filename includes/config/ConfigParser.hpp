#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "ServerConfig.hpp"

#define ROOT 1
#define CLIENT_MAX_BODY_SIZE 2
#define RETURN 3
#define AUTOINDEX 4
#define INDEX 5

class ConfigParser {

private:

const char *argv;
std::vector<ServerConfig*> serverConfigVector;
int server_in;
bool location_executed;
std::string error_line;
std::string nbr_line;

public:
	ConfigParser();
	ConfigParser(const char *argv);
	~ConfigParser();
	ConfigParser(ConfigParser const &configParser);

    std::vector<ServerConfig*> getServerConfigVector();

	void proccess_input (void);

	//Parsing Aux
    bool check_config_struct(std::stringstream& ss);

    int parse_var(std::string &token, std::stringstream& ss, int macro);
    bool parse_var_add(int macro, std::string &token);

	bool checkServer(std::string token, std::stringstream& ss);
    bool parse_listen(std::string &token, std::stringstream& ss);
    int parse_server_name(std::string &token, std::stringstream& ss);
    int parse_error_page(std::string &token, std::stringstream& ss);
    int parse_location(std::string &token, std::stringstream& ss);
    int verify_error_code(std::string &token, int &flag_code, int &flag_page);
    int parse_limit_except(std::string &token, std::stringstream& ss);
    bool check_valid_method(std::string token);
};