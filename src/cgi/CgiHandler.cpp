#include "cgi/CgiHandler.hpp"


void CgiHandler::initCgi_Env(HttpRequest& request, HttpServer& server, RouteHandler &route)
{
    this->cgi_Env["SERVER_NAME"] = request.getHost();
    this->cgi_Env["SERVER_SOFTWARE"] = "webserv/1.0";
    this->cgi_Env["CONTENT_TYPE"]=request.getHeaders().find("Content-Type")->second;
    this->cgi_Env["GATEWAY_INTERFACE"]="CGI/1.1";
    this->cgi_Env["REMOTE_ADDR"]=server.getHost();
    this->cgi_Env["REMOTE_PORT"]=server.getPort();
    this->cgi_Env["REQUEST_METHOD"]=request.getMethod();
    this->cgi_Env["SCRIPT_NAME"] = route.getLocation().getCgiPath();
    this->cgi_Env["QUERY_STRING"]=request.getQueryString();

/*         "SCRIPT_NAME=/script_name",  // replace with the actual script name
        "QUERY_STRING=parameter1=value1&parameter2=value2",
        // Add any other CGI environment variables your script might need
 */
}

void CgiHandler::executeCgi_Script()
{
    fd = open();
    pipe(tubes);
    if ((cgi_pid = fork())== 0) // child process
    {
        close(tubes[1]);  
        dup2(tubes[0], 0);   
        execve(args[0], args, env);
    }
}
void             ft::CGI::execute(HttpRequest& request, int fd, )
{
    pid_t   pid;
    int     pipe_in[2]
    int     pipe_out[2];
    int     status;

    if (pipe(pipe_in) < 0)
    {
        std::cerr << "pipe failed\n";
        response.setStatusCode("500");
        return ;
    }
    if (pipe(pipe_out) < 0)
    {
        std::cerr << "pipe failed\n";
        close(pipe_in[0]);
        close(pipe_in[1]);
        response.setStatusCode("500");
        return ;
    }
    pid = fork();
    if (pid == 0)
    {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        write(pipe_in[1], request.getBody().c_str(), atoi(this->cgi_Env["CONTENT_LENGTH"].c_str()));
        close(pipe_in[0]);
        //close(pipe_in[1]);
        //close(pipe_out[0]);
        close(pipe_out[1]);
        status = execve(_argv[0], _argv, _cenv);
        response.setStatusCode("500");
        exit(status);
    }
    else if (pid > 0){

        close(pipe_in[1]);
        waitpid(pid, &status, 0);
        close(pipe_out[1]);
        if (status < 0)
        {
            close(pipe_out[0]);
            close(pipe_in[0]);
            return ;
        }
    }
}