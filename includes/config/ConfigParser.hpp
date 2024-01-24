#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "ServerConfig.hpp"
#include <unistd.h>

#define LISTEN 1
#define ROOT 2
#define CLIENT_MAX_BODY_SIZE 3
#define RETURN 4
#define AUTOINDEX 5
#define INDEX 6
#define CGI_PATH 7
#define CGI_EXT 8
#define UPLOAD_STORE 9

class ConfigParser {

private:

const char *argv;
std::vector<ServerConfig*> serverConfigVector;
int server_in;
bool executed;
std::string error_line;
std::string nbr_line;

public:
	ConfigParser();
	ConfigParser(const char *argv);
	~ConfigParser();
	ConfigParser(ConfigParser const &configParser);

    std::vector<ServerConfig*> getServerConfigVector();

	bool proccess_input (void);

	//Parsing Aux
    bool check_config_struct(std::stringstream& ss);

    int parse_var(std::string &token, std::stringstream& ss, int macro);
    bool parse_var_add(int macro, std::string &token);

	bool checkServer(std::string token, std::stringstream& ss);
    bool parse_listen(std::string &token, std::stringstream& ss);
    bool parse_host_port(std::string& host_port, char sep);
    int parse_server_name(std::string &token, std::stringstream& ss);
    int parse_error_page_path(std::string &token, std::stringstream& ss);
    int parse_location(std::string &token, std::stringstream& ss);
    int verify_error_code(std::string &token, int &flag_code, int &flag_page);
    int parse_limit_except(std::string &token, std::stringstream& ss);
    bool check_valid_method(std::string token);
    bool hasLocationWithName(ServerConfig *serverConfig, std::string name);
	bool hasRepeatedPorts(std::vector<ServerConfig*> serverConfigVector);
	void freeServerConfigVector();
};