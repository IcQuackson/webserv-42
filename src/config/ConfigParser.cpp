#include "../../includes/config/ConfigParser.hpp"

ConfigParser::ConfigParser() {ConfigParser("default path");}

ConfigParser::ConfigParser(char *argv):argv(argv), server_in(0) {}

ConfigParser::~ConfigParser() {}

bool ConfigParser::checkServer(std::string token, std::stringstream& ss) 
{
    std::string next_token;
    ss >> next_token;
    if (token == "server" && next_token == "{")
    {
        this->server_in = 1;
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
            while (ss.get(ch) && ch != '\n') {}
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
    removeComments(line, '#', '\n');
    while (line >> token)
    {
        if (!this->server_in)
        {
            if (!checkServer(token, line))
                throw std::runtime_error("Error: Invalid input near token :" + token);
            continue;
        }
        if (find_port_host(token, line))
            continue;
        
    }
}
