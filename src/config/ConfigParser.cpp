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

bool ConfigParser::parse_listen(std::string &token, std::stringstream& ss)
{
    std::string next_token;
    if (token == "listen")
    {
        ss >> next_token;
        if (!isDigits(next_token))
            return (0);
        double port = std::atof(next_token.c_str());
        if (port < 1 || port > 65535)
            return (0);
        this->serverConfigVector.back()->addPort(port);
        if (!(next_token[next_token.length() - 1] == ';'))
        {
            ss >> next_token;
            if (next_token != ";")
                return (0);
        }
        return (1);
    }
    return (0);
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
        return (1);
    }
    return (-1);
}

int ConfigParser::parse_root(std::string &token, std::stringstream& ss)
{
    if (token == "root")
    {
        ss >> token;
        if (token == ";")
            return (0);
        if (token[token.length() - 1] == ';')
        {
            token.erase(token.size() - 1);
            this->serverConfigVector.back()->setRoot(token);
            ss >> token;
            return (1);
        }
        this->serverConfigVector.back()->setRoot(token);
        ss >> token;
        if (token == ";")
        {
            ss >> token;
            return (1);
        }
        return (0);
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
        this->serverConfigVector.back()->addError_page(token);
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
        return (1);
    }
    return (-1);
}

int ConfigParser::parse_client_max_body_size(std::string &token, std::stringstream& ss)
{
    if (token == "client_max_body_size")
    {
        ss >> token;
        if (token == ";")
            return (0);
        if (token[token.length() - 1] == ';')
        {
            token.erase(token.size() - 1);
            this->serverConfigVector.back()->setClient_max_body_size(token);
            ss >> token;
            return (1);
        }
        this->serverConfigVector.back()->setClient_max_body_size(token);
        ss >> token;
        if (token == ";")
        {
            ss >> token;
            return (1);
        }
        return (0);
    }
    return (-1);
}


int ConfigParser::parse_location(std::string &token, std::stringstream& ss)
{
    std::string next_token;
    if (token == "location")
    {
        ss >> token;
        if (token[0] == '/')
            this->serverConfigVector.back()->addLocation(new Location(token));
        ss >> token;
        if (token != "{")
            return (0);
        else
        {
            while (token != "}")
            {
                if (!parse_root(token, ss))
                    throw std::runtime_error("Error: Invalid input near token 3: " + token);
                ss >> token;
            };
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

std::string see_next_token(std::stringstream& line)
{
    std::streampos startPosition = line.tellg();
    std::string next_token;
    line >> next_token;
    line.seekg(startPosition);
    return (next_token);
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

bool check_semiCollon_between_braces(std::stringstream& ss) 
{
    std::string line;
    bool insideBraces = 0;

    while (std::getline(ss, line)) 
    {
        size_t openBracePos = line.find('{');
        size_t closeBracePos = line.find('}');

        if (openBracePos != std::string::npos)
        {
            insideBraces = 1;
            continue;
        }
        if (closeBracePos != std::string::npos)
            continue;
        if (insideBraces) 
        {
            // Check if the line ends with a semicolon
            if (!line.empty() && !endsWithSemicolon(line))
                return (0);
        }
        if (closeBracePos != std::string::npos)
            insideBraces = 0;
    }
    return (1);
}

void ConfigParser::proccess_input()
{
    std::ifstream ifile;
    std::stringstream line;
    std::string buff;
    std::string token;
    std::string next_token;

    ifile.open(argv);
    if (!(ifile.is_open()))
        throw std::runtime_error("Error: could not open file.");
    line << ifile.rdbuf();
    ifile.close();
    buff = line.str();
    std::stringstream braces(buff);
    removeComments(line, '#', '\n');
    if (!check_semiCollon_between_braces(braces))
        throw std::runtime_error("Error: Lines not finishing in semi collon iniside braces");
    if (!checkBraces(line.str()))
        throw std::runtime_error("Error: braces are not correctly placed.");
    while (line >> token)
    {
        if (!this->server_in)
        {
            if (!checkServer(token, line))
                throw std::runtime_error("Error: Invalid input near token 1: " + token);
            continue;
        }
        if (!parse_listen(token, line))
            throw std::runtime_error("Error: Invalid input near token 2: " + token);
        else if (see_next_token(line) == "listen")
            continue;
        line >> token;
        try
        {
            if (!parse_server_name(token, line))
                throw std::runtime_error("Error: server_name parsing: " + token);
            if (!parse_root(token, line))
                throw std::runtime_error("Error: root parsing: " + token);
            if (!parse_error_page(token,line))
                throw std::runtime_error("Error: error_page parsing: " + token);
            if (!parse_client_max_body_size(token,line))
                throw std::runtime_error("Error: client_max_body_size parsing: " + token);
            parse_location(token, line);
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            break;
        }
        if (token == "}")
        {
            this->server_in = 0;
            break;
        }
        
    }
}

std::vector<ServerConfig*> ConfigParser::getServerConfigVector()
{
    return this->serverConfigVector;
}