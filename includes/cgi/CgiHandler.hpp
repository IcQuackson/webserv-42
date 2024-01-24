#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include "core/HttpServer.hpp"
#include "core/RouteHandler.hpp"
#include "core/HttpRequest.hpp"
#include "config/Location.hpp"
#include "config/ServerConfig.hpp"


class CgiHandler {

private:
    std::map<std::string, std::string> cgi_Env;
    std::string script_path;

public:
    int	fd_in[2];
	int	fd_out[2];

    void exec_cgi_py(HttpRequest& request, HttpResponse& response, RouteHandler& routehandler, int type);
    void initCgi_Env(RouteHandler& route, HttpRequest& request);
    void execute_script(HttpRequest& request, HttpResponse& response, RouteHandler& route, int type);
    std::string extract_filename(std::string content_disp);
	
};