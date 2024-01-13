#include "cgi/CgiHandler.hpp"

/* std::vector<std::string> Client::createEnvVars(const std::string& serverRoot, std::string uri, const location_t& targetLocation) {
    std::vector<std::string> envVars;

    if (uri.at(0) == '/')
        uri.erase(0, 1);

    envVars.push_back("SCRIPT_FILENAME=" + (serverRoot + targetLocation.cgi_path + uri));

    if (headers.count("Content-Length") > 0) {
        envVars.push_back("CONTENT_LENGTH=" + headers["Content-Length"]);
    }

    envVars.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
    envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envVars.push_back("REQUEST_METHOD=" + this->method);
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envVars.push_back("SERVER_SOFTWARE=42_Webserv/1.0");

    return envVars;
} */

void CgiHandler::initCgi_Env(RouteHandler& route, HttpRequest& request)
{
    (void) route;
    this->cgi_Env["SCRIPT_FILENAME"] = route.getLocation().getRoot() + route.getLocation().getCgiPath();
    this->cgi_Env["SERVER_NAME"] = request.getHost();
    this->cgi_Env["SERVER_SOFTWARE"] = "webserv-42/1.0";
    std::map<std::string, std::string>::iterator it = request.getHeaders().find("Content-Type");
    if (it != request.getHeaders().end())
        this->cgi_Env["CONTENT_TYPE"]=request.getHeaders().find("Content-Type")->second;
    it = request.getHeaders().find("Content-Lenght");
    if (it != request.getHeaders().end())
        this->cgi_Env["CONTENT_LENGTH"] = request.getHeaders().find("Content-Lenght")->second;;
    this->cgi_Env["GATEWAY_INTERFACE"]="CGI/1.1";
    this->cgi_Env["REMOTE_ADDR"]=route.getServerConfig().getHost();
    this->cgi_Env["REMOTE_PORT"]=route.getServerConfig().getPort();
    this->cgi_Env["REQUEST_METHOD"]=request.getMethod();
    this->cgi_Env["QUERY_STRING"]=request.getQueryString();

}
/*         "SCRIPT_NAME=/script_name",  // replace with the actual script name
        "QUERY_STRING=parameter1=value1&parameter2=value2",
        // Add any other CGI environment variables your script might need
 
}*/
/* 
    this->cgi_path = getScriptFileName(envVars);
    if (cgi_path.empty())
        throw CGIException("No SCRIPT_FILENAME provided");
    this->cgi_ext = cgi_ext;

    size_t dotPos = cgi_path.find_last_of('.');
    if (dotPos == std::string::npos)
        throw CGIException("No extension. Expected \"" + cgi_ext + "\"");

    std::string ext = cgi_path.substr(dotPos);
    if (ext != cgi_ext)
        throw CGIException("Invalid extension \"" + ext + "\" expected \"" + cgi_ext + "\"");

    if (access(this->cgi_path.c_str(), F_OK) != 0)
        throw CGIException("Invalid CGI file");
    this->createArgvAndEnvp(envVars);
    this->runScript(); */

void CgiHandler::exec_cgi_py(HttpRequest& request, HttpResponse& response, RouteHandler& route, int type)
{
    std::string full_path = route.getLocation().getCgiPath();
    size_t lastSlashPos = request.getResource().find_last_of('/');
    std::string scriptName = route.getLocation().getCgiPath().substr(lastSlashPos + 1);

    std::cout << "filename:" << scriptName << std::endl;
    if(scriptName.empty())
    {
        std::cerr << "Error: Empty script name." << std::endl;
        response.setStatusCode("400");
        return ;
    }
    
    size_t dotPos = scriptName.find_last_of('.');
    if (dotPos == std::string::npos)
    {
        std::cerr << "Error: No file extension found." << std::endl;
        response.setStatusCode("400");
        return ;
    }
    
    std::string file_ext = scriptName.substr(dotPos);
    if (file_ext != route.getLocation().getCgiExtension())
    {
        std::cerr << "Error: Invalid file extension." << std::endl;
        response.setStatusCode("400");
        return ;
    }

    if (access(full_path.c_str(), F_OK) != 0)
    {
        std::cerr << "Error: File not found" << std::endl;
        response.setStatusCode("404");
        return ;
    }
    initCgi_Env(route, request);
    execute_script(request,response,route,type);
}

void    CgiHandler::execute_script(HttpRequest& request, HttpResponse& response, RouteHandler& route, int type)
{
    pid_t   pid;
    int     pipes[2];
    int     status;

    if (pipe(pipes) < 0)
    {
        std::cerr << "pipe failed\n";
        response.setStatusCode("500");
        return ;
    }

    if (type)
    {
        fcntl(pipes[1], F_SETPIPE_SZ, request.getBody().length());
        write(pipes[1], request.getBody().c_str(), request.getBody().length());
        close(pipes[1]);
    }
    else
    {
        write(pipes[1], request.getBody().c_str(), 0);
        close(pipes[1]);
    }
    pid = fork();
    if (pid == 0)
    {
        dup2(pipes[0], STDIN_FILENO);
        close(pipes[0]);

        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);
        
        std::string python_exe = "/usr/bin/python";

        char **argv = new char*[4];
        argv[0] = strdup("python");
        argv[1] = strdup(route.getLocation().getCgiPath().c_str());
        if (type)
            argv[2] = strdup(route.getLocation().getUploadPath().c_str());
        else
            argv[2] = strdup("");
        argv[3] = NULL;

        char **env_vars = new char*[this->cgi_Env.size() + 1];

        size_t i = 0;
        std::map<std::string, std::string>::iterator it;
        for (it = this->cgi_Env.begin(); it != this->cgi_Env.end(); it++, i++) {
            env_vars[i] = strdup(it->second.c_str());
        }
        env_vars[i] = NULL;
    
        status = execve("/usr/bin/python", argv, env_vars);
        perror("execve failed");
        response.setStatusCode("500");
        exit(status);
    }
    else
    {
        close(pipes[0]);
        close(pipes[1]);
        waitpid(pid, &status, 0);
    }
}