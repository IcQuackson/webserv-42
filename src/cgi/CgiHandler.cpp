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