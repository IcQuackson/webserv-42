/* How to Programming CGI?

[Step1] Make process by using fork( ). Also open pipes.

[Step 2] Execute CGI in child process.

fd = open(//temporary file to record cgi result);
pipe(tubes);
if ((cgi_pid = fork())== 0) // child process
{
  close(tubes[1]);  
  dup2(tubes[0], 0);   
  execve(args[0], args, env);
}

    args[0] has to be a path of cgi program or cgi script. Such as “/usr/local/bin/php-cgi”
    args[1] has to be a path of cgi file . If it script it has to be NULL.
    args[2] == NULL
    env has parsed request and some more variables according to RFC3875 
Meta-variables with names beginning with "HTTP_" contain values read    from the client request header fields, if the protocol used is HTTP.    
The HTTP header field name is converted to upper case, has all    occurrences of "-" replaced with "_" and has "HTTP_" prepended to    give the meta-variable name.

[STEP 3] Pass requested data through pipe in Parent Process.

CGI will give result to web-server and web-server will make response based on this result.*/


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
	
};