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
	// Create populate env variables that need to be passed to the CGI script
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
    this->cgi_Env["REQUEST_METHOD"]=request.getMethod();
    this->cgi_Env["QUERY_STRING"]=request.getQueryString();

}

void CgiHandler::exec_cgi_py(HttpRequest& request, HttpResponse& response, RouteHandler& route, int type)
{
    std::string full_path = route.getLocation().getRoot() + route.getLocation().getCgiPath();
    std::cout << "full:" << full_path << std::endl;
    size_t lastSlashPos = request.getResource().find_last_of('/');
    //if (type)
    //    lastSlashPos += 1;
    std::string scriptName = route.getLocation().getCgiPath().substr(lastSlashPos + 1);

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
        std::cout << "Error: cgi_handler" << std::endl;
        return ;
    }
    initCgi_Env(route, request);
    execute_script(request, response, route, type);
}

std::string CgiHandler::extract_filename(std::string content_disp)
{
    size_t filenamePos = content_disp.find("filename=");
    if (filenamePos != std::string::npos) {
        // Extract the substring starting from the position after "filename=" until the closing quote
        size_t startQuotePos = content_disp.find("\"", filenamePos);
        size_t endQuotePos = content_disp.find("\"", startQuotePos + 1);

        if (startQuotePos != std::string::npos && endQuotePos != std::string::npos) {
            std::string filename = content_disp.substr(startQuotePos + 1, endQuotePos - startQuotePos - 1);
            return (filename);
        } 
        else
            return "";
    } 
    else 
        return "";
}

void    CgiHandler::execute_script(HttpRequest& request, HttpResponse& response, RouteHandler& route, int type)
{
    pid_t   pid;
    int     pipes[2];
    int     status;
    std::string filename;

    if (request.getHeaders().count("Content-Disposition") > 0)
    {
        filename = extract_filename(request.getHeaders().find("Content-Disposition")->second);
        if (filename.empty())
        {
            std::cerr << "File Name Not Extracted" << std::endl;
            response.setStatusCode("500");
            return ;
        }
    }
    if (pipe(pipes) < 0)
    {
        std::cerr << "pipe failed" << std::endl;
        response.setStatusCode("500");
        return ;
    }
	std::cout << "GET" << std::endl;
	std::cout << "cgi_path: " << route.getLocation().getRoot() + route.getLocation().getCgiPath() << std::endl;
	
    pid = fork();
    if (pid == 0)
    {
        dup2(pipes[0], STDIN_FILENO);
        close(pipes[0]);

        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);
        
        char **argv = new char*[7];
        argv[0] = strdup("python");

        // If POST
        if (type)
        {
            argv[1] = strdup((route.getLocation().getRoot() + route.getLocation().getCgiPath()).c_str());
            argv[2] = strdup((route.getLocation().getRoot() + route.getLocation().getUploadPath()).c_str());
            argv[3] = strdup(filename.c_str());
            argv[4] = strdup(request.getBody().c_str());
			argv[5] = strdup("POST");
        }
        else
        {
			argv[1] = strdup((route.getLocation().getRoot() + route.getLocation().getCgiPath()).c_str());
            argv[2] = strdup("");
            argv[3] = strdup("");
            argv[4] = strdup("");
			argv[5] = strdup("GET");
        }
        argv[6] = NULL;

        char **env_vars = new char*[this->cgi_Env.size() + 1];

        size_t i = 0;
        std::map<std::string, std::string>::iterator it;
        for (it = this->cgi_Env.begin(); it != this->cgi_Env.end(); it++, i++) {
            env_vars[i] = strdup(it->second.c_str());
        }
        env_vars[i] = NULL;
        status = execve("/usr/bin/python3", argv, env_vars);
        perror("execve failed");
        response.setStatusCode("500");
        exit(status);
    }
    else
    {
        close(pipes[1]);
        char buffer[2048]; 
        int readBytes = read(pipes[0], buffer, 2048);
        close(pipes[0]);
        if (readBytes < 0)
        {
            std::cerr << "parent process: read failed" << std::endl;
            response.setStatusCode("500");
            return ;
        }
        buffer[readBytes] = '\0';

        std::string body(buffer);
        pid_t wpid;
        wpid = waitpid(pid, &status, WNOHANG);
        if (wpid == 0) {
            kill (pid, SIGKILL);
        }
        // Child process is still running
        if (wpid == -1) {
            // waitpid failed
            perror("waitpid");
        } else if (WIFEXITED(status)) {
            // Child process exited normally
            int exit_status = WEXITSTATUS(status);
            std::cout << "Child exited with status: " << exit_status << std::endl;
            if (exit_status != 0) {
                response.setStatusCode("500");
                return;
            }
        } else if (WIFSIGNALED(status)) {
            // Child process was terminated by a signal
            int term_signal = WTERMSIG(status);
            std::cout << "Child was terminated by signal: " << term_signal << std::endl;
        }
        response.setBody(body);
    }
}