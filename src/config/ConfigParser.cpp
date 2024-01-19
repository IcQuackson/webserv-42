#include "../../includes/config/ConfigParser.hpp"

ConfigParser::ConfigParser() {ConfigParser("default path");}

ConfigParser::ConfigParser(const char *argv):argv(argv), server_in(0) {}

ConfigParser::ConfigParser(ConfigParser const &configParser)
{
    *this = configParser;
}

ConfigParser::~ConfigParser() {}

bool ConfigParser::checkServer(std::string token, std::stringstream& ss) 
{
    std::string next_token;
    ss >> next_token;
    if (token == "server" && next_token == "{")
    {
        this->server_in = 1;
        ServerConfig* newServerConfig = new ServerConfig();
        this->serverConfigVector.push_back(newServerConfig);
        return (1);
    }
    return (0);
}

void removeComments(std::stringstream& ss, char startChar, char endChar) 
{
    std::stringstream result;
    char ch = '\0';

    while (ss.get(ch)) 
    {
        if (ch == startChar) {
            // Skip characters until the next newline
            while (ss.get(ch) && ch != endChar) {}
        } 
        else 
        {
            // Append the character to the result
            result.put(ch);
        }
    }
    // Clear the original stringstream
    ss.str("");
    ss.clear();
    ss << result.rdbuf();  // Copy the content from result to the original stringstream
}

bool isDigits(std::string& str) 
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) 
    {
        if (!isdigit(*it))
        {
            if (it == str.end() - 1 && *it == ';' && it != str.begin())
                continue;
            return (0);
        }
    }
    return (1); 
}

bool isValidIPAddress(std::string& ipAddress) 
{
    std::vector<int> octets;
    std::istringstream ss(ipAddress);
    std::string octet;

    while (std::getline(ss, octet, '.')) 
    {
        int value = std::atoi(octet.c_str());
        if (value < 0 || value > 255)
            return 0;
        octets.push_back(value);
    }
    return (octets.size() == 4);
}

bool check_host_port(std::string &token, int is_port)
{
    if (is_port)
    {
        if (!isDigits(token))
            return (0);
        double port = std::atof(token.c_str());
        if (port < 1 || port > 65535)
            return (0);
    }
    else
    {
        if (!isValidIPAddress(token))
            return (0);
    }
    return (1);
}

bool ConfigParser::parse_host_port(std::string& host_port, char delimiter)
{
    std::string::iterator sep = std::find(host_port.begin(), host_port.end(), delimiter);
    std::string host;
    std::string port;

    std::size_t start_pos = std::distance(host_port.begin(), sep);
    std::size_t sep_pos = std::distance(sep + 1, host_port.end());

    if (sep == host_port.begin())
        return (0);
    if (sep != host_port.end())
    {
        host = host_port.substr(start_pos, sep_pos - start_pos);
        port = host_port.substr(sep_pos - start_pos + 1);
    }
    else
        port = host_port;
    if (!port.empty() && check_host_port(port, 1))
        this->serverConfigVector.back()->setPort(std::atof(port.c_str()));
    else
        return (0);
    if (host.empty() || host == "localhost")
    {
        this->serverConfigVector.back()->setHost("127.0.0.1");
        return (1);
    }
    else if (!check_host_port(host, 0))
        return (0);
    this->serverConfigVector.back()->setHost(host);
    return (1);        
}

int ConfigParser::parse_server_name(std::string &token, std::stringstream& ss)
{
    if (token == "server_name")
    {
        ss >> token;
        if (token == ";")
            return (0);
        while (token != ";")
        {
            if (token[token.length() - 1] == ';')
            {
                token.erase(token.size() - 1);
                this->serverConfigVector.back()->addServer_name(token);
                break;
            }
            else
                this->serverConfigVector.back()->addServer_name(token);
            ss >> token;
        }
        ss >> token;
        this->executed = 1;
        return (1);
    }
    return (-1);
}


int ConfigParser::verify_error_code(std::string &token, int &flag_code, int &flag_page)
{
    if (!isDigits(token) && !flag_code && !flag_page)
        return (0);
    if (isDigits(token))
    {
        if (flag_page)
            return (0);
        double error_code = std::atof(token.c_str());
        if (error_code < 1 || error_code > 599)
            return (0);
        this->serverConfigVector.back()->addError_code(error_code);
        flag_code++;
    }
    else
    {
        this->serverConfigVector.back()->setErrorPage(token);
        flag_page++;
    }
    return (1);
} 

int ConfigParser::parse_error_page(std::string &token, std::stringstream& ss)
{
    if (token == "error_page")
    {
        ss >> token;
        if (token == ";")
            return (0);
        int flag_code = 0;
        int flag_page = 0;
        while (token != ";")
        {
            if (token[token.length() - 1] == ';')
            {
                token.erase(token.size() - 1);
                if (!verify_error_code(token, flag_code, flag_page))
                    return (0);
                break;
            }
            else
            {
                if (!verify_error_code(token, flag_code, flag_page))
                    return (0);
            }
            ss >> token;
        }
        ss >> token;
        this->executed = 1;
        return (1);
    }
    return (-1);
}

bool ConfigParser::parse_var_add(int macro, std::string &token)
{
    if (macro == LISTEN)
    {
        if (!parse_host_port(token, ':'))
            return (0);
    }
    else if (macro == ROOT)
        this->serverConfigVector.back()->getLocations().back()->setRoot(token);
    else if (macro == CLIENT_MAX_BODY_SIZE)
    {
        if (!isDigits(token))
            return (0);
        this->serverConfigVector.back()->setClient_max_body_size(token);
    }
    else if (macro == RETURN)
        this->serverConfigVector.back()->getLocations().back()->setRedirection(token);
    else if (macro == AUTOINDEX)
    {
        bool directoryListing;
        if (token == "on")
            directoryListing = 1;
        else if (token == "off")
            directoryListing = 0;
        else
            return (0);
        this->serverConfigVector.back()->getLocations().back()->setDirectoryListing(directoryListing);
    }
    else if (macro == INDEX)
        this->serverConfigVector.back()->getLocations().back()->setDefaultFile(token);
    else if (macro == CGI_PATH)
        this->serverConfigVector.back()->getLocations().back()->setCgiPath(token);
    else if (macro == CGI_EXT)
        this->serverConfigVector.back()->getLocations().back()->setCgiExtension(token);
    else if (macro == UPLOAD_STORE)
    {
        this->serverConfigVector.back()->getLocations().back()->setAcceptUploads(true);
        std::string UploadEndpoint = this->serverConfigVector.back()->getLocations().back()->getPath();
        this->serverConfigVector.back()->getLocations().back()->setUploadEndpoint(UploadEndpoint);
        this->serverConfigVector.back()->getLocations().back()->setUploadPath(token);
    }
    return (1);
}

int ConfigParser::parse_var(std::string &token, std::stringstream& ss, int macro)
{
    if ((token == "listen" && macro == LISTEN) || (token == "root" && macro == ROOT) 
        || (token == "client_max_body_size" && macro == CLIENT_MAX_BODY_SIZE) 
        || (token == "return" && macro == RETURN) || (token == "autoindex" && macro == AUTOINDEX) 
        || (token == "index" && macro == INDEX) || (token == "cgi_path" && macro == CGI_PATH) 
        || (token == "cgi_ext" && macro == CGI_EXT) || (token == "upload_store" && macro == UPLOAD_STORE))
    {
        ss >> token;
        if (token == ";")
            return (0);
        if (token[token.length() - 1] == ';')
        {
            token.erase(token.size() - 1);
            if(!parse_var_add(macro, token))
                return (0);
            if (macro != LISTEN)
                ss >> token;
            this->executed = 1;
            return (1);
        }
        else
        {
            if(!parse_var_add(macro, token))
                return (0);
            ss >> token;
            this->executed = 1;
            if (token == ";")
                return (1);
            return (0);
        }
    }
    return (-1);
}

bool ConfigParser::check_valid_method(std::string token)
{
    const char *methods_arr[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
    size_t size = sizeof(methods_arr) / sizeof(methods_arr[0]);
    std::vector<std::string> methods_vector(methods_arr, methods_arr + size);

    std::vector<std::string>::iterator it = std::find(methods_vector.begin(), methods_vector.end(), token);

    if (it == methods_vector.end())
        return (0);
    
    methods_vector = this->serverConfigVector.back()->getLocations().back()->getMethods();
    it = std::find(methods_vector.begin(), methods_vector.end(), token);
    if (it != methods_vector.end())
        return (0);
    return (1);
}

int ConfigParser::parse_limit_except(std::string &token, std::stringstream& ss)
{
    if (token == "limit_except")
    {
        ss >> token;
        if (token == ";")
            return (0);
        while (token != ";")
        {
            if (token[token.length() - 1] == ';')
            {
                token.erase(token.size() - 1);
                if (!check_valid_method(token))
                    return (0);
                this->serverConfigVector.back()->getLocations().back()->addMethod(token);
                break;
            }
            else
            {
                if (!check_valid_method(token))
                    return (0);
                this->serverConfigVector.back()->getLocations().back()->addMethod(token);
            }
            ss >> token;
        }
        ss >> token;
        this->executed = 1;
        return (1);
    }
    return (-1);
}

std::string see_next_token(std::stringstream& line)
{
    std::streampos startPosition = line.tellg();
    std::string next_token;
    line >> next_token;
    line.seekg(startPosition);
    return (next_token);
}

bool ConfigParser::hasLocationWithName(ServerConfig* serverConfig, std::string name) {
    std::vector<Location*> locations = serverConfig->getLocations();
    for (std::vector<Location*>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if ((*it)->getPath() == name)
            return (1);
    }
    return (0);
}

int ConfigParser::parse_location(std::string &token, std::stringstream& ss)
{
    std::string next_token;
    if (token == "location")
    {
        ss >> token;
        // TODO root
        if (token[0] == '/')
        {
            if (hasLocationWithName(this->serverConfigVector.back(), token))
                throw std::runtime_error("Error: location already exists: " + token);
            Location *location = new Location(token);
            int maxClientBodySize = std::atoi(this->serverConfigVector.back()->getClient_max_body_size().c_str());
            if (maxClientBodySize > 2048 || maxClientBodySize < 0)
                throw std::runtime_error("Error: Invalid client_max_body_size > 2048");
            location->setClientBodySize(maxClientBodySize);
            this->serverConfigVector.back()->addLocation(location);
        }
        ss >> token;
        if (token != "{")
            return (0);
        else
        {
            ss >> token;
            int loops = 0;
            while (token != "}")
            {
                this->executed = 0;
                if (!parse_limit_except(token, ss))
                    throw std::runtime_error("Error: Invalid input near token 3: " + token);
                if (!parse_var(token, ss, RETURN))
                    throw std::runtime_error("Error: Invalid input near token 4: " + token);
                if (!parse_var(token, ss, ROOT))
                    throw std::runtime_error("Error: Invalid input near token 5: " + token);
                if (!parse_var(token, ss, AUTOINDEX))
                    throw std::runtime_error("Error: Invalid input near token 6: " + token);
                if (!parse_var(token, ss, INDEX))
                    throw std::runtime_error("Error: Invalid input near token 7: " + token);
                if (!parse_var(token, ss, CGI_PATH))
                    throw std::runtime_error("Error: Invalid input near token 8: " + token);
                if (!parse_var(token, ss, CGI_EXT))
                    throw std::runtime_error("Error: Invalid input near token 9: " + token);
                if (!parse_var(token, ss, UPLOAD_STORE))
                    throw std::runtime_error("Error: Invalid input near token 10: " + token);
                if (!this->executed)
                    throw std::runtime_error("Error: Invalid input near token 11: " + token);
                loops++;
                if (loops > 5 || token == ";")
                {
                    loops = 0;
                    if(token != "}")
                        ss >> token;
                }
            }
            if (see_next_token(ss) != "server")
                ss >> token;
            if (token == "location")
                parse_location(token, ss);
        }
        return (1);
    }
    return (-1);
}

bool checkBraces (std::string file)
{
    std::string token;
    std::stringstream ss(file);
    int braces_open = 0;

    while (ss >> token)
    {
        if (token == "{")
            braces_open++;
        else if (token == "}")
        {
            if (!braces_open)
                return (0);
            else
                braces_open--;
        }
    }
    if (braces_open)
        return (0);
    return (1);
}

bool endsWithSemicolon(std::string& str) 
{
    // Find the index of the last non-whitespace character
    size_t lastNonWhitespace = str.find_last_not_of(" \t");

    // Check if the string is not empty and the last non-whitespace character is a semicolon
    if (!str.empty() && lastNonWhitespace != std::string::npos && str[lastNonWhitespace] == ';')
        return (1);
    return (0);
}

bool isWhitespace(std::string& str) 
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) 
    {
        if (!std::isspace(*it))
            return (0);
    }
    return (1);
}

std::string extractFirstWord(std::string& input) 
{
    std::istringstream ss(input);
    std::string firstWord;
    
    ss >> firstWord;
    
    return firstWord;
}

bool ConfigParser::check_config_struct(std::stringstream& ss) 
{
    std::string line;
    bool insideBraces = 0;
    int nbr_line = 1;
    std::ostringstream nbr_stream;

    while (std::getline(ss, line)) 
    {
        this->error_line = line;
        nbr_line++;
        nbr_stream.str("");
        nbr_stream.clear();
        nbr_stream << nbr_line;
        this->nbr_line = nbr_stream.str();
        size_t openBracePos = line.find('{');
        size_t closeBracePos = line.find('}');

        if (openBracePos != std::string::npos)
        {
            insideBraces = 1;
            continue;
        }
        if (closeBracePos != std::string::npos)
        {
            insideBraces = 0;
            continue;
        }
        if (insideBraces) 
        {
            // Check if the line ends with a semicolon
            if (!line.empty() && !isWhitespace(line) && !endsWithSemicolon(line))
                return (0);
        }
        if (!isWhitespace(line) && extractFirstWord(line) != "server" && !insideBraces)
            return (0);
    }
    return (1);
}

bool ConfigParser::proccess_input()
{
    std::ifstream ifile;
    std::stringstream line;
    std::string buff;
    std::string token;
    std::string next_token;

    ifile.open(argv);

    if (!(ifile.is_open()))
    {
        throw std::runtime_error("Error: could not open file.");
    }

    line << ifile.rdbuf();
    ifile.close();
    removeComments(line, '#', '\n');
    buff = line.str();
    std::stringstream tmp_line(buff);

    if (!checkBraces(buff))
    {
        throw std::runtime_error("Error: braces are not correctly placed.");
    }

    // Check for ; at the end of each configuration
    if (!check_config_struct(tmp_line))
    {
        throw std::runtime_error("Error: Invalid argument in line nbr " + this->nbr_line + ": " + this->error_line);
    }

    // Parse the configuration
    while (line >> token)
    {
        // Check if the token is a server
        if (!this->server_in)
        {
            if (!checkServer(token, line))
                throw std::runtime_error("Error: Invalid input near token 1: " + token);
            continue;
        }
        if (!parse_var(token, line, LISTEN))
            throw std::runtime_error("Error: Invalid input near token 2: " + token);
        /* else if (see_next_token(line) == "listen")
            continue; */
        line >> token;

        // Stores configurations
        try
        {
            this->executed = 1;
            while (this->executed)
            {
                this->executed = 0;
                if (!parse_server_name(token,  line))
                    throw std::runtime_error("Error: server_name parsing: " + token);
                /* if (!parse_root(token, line))
                    throw std::runtime_error("Error: root parsing: " + token); */
                if (!parse_error_page(token, line))
                    throw std::runtime_error("Error: error_page parsing: " + token);
                if (!parse_var(token, line, CLIENT_MAX_BODY_SIZE))
                    throw std::runtime_error("Error: client_max_body_size parsing: " + token);
                parse_location(token, line);
                if (token == ";")
                    line >> token; 
            }
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return (0);
        }
        if (token == "}")
        {
            this->server_in = 0;
            if (see_next_token(line) == "server")
                continue;
            break;
        }
        else
            throw std::runtime_error("Error: Invalid input near token 12: " + token);
        
    }
    return (1);
}

std::vector<ServerConfig*> ConfigParser::getServerConfigVector()
{
    return this->serverConfigVector;
}