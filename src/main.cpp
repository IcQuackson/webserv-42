#include "core/httpServer.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "core/HttpStatusCode.hpp"

#include <vector>

int main(int argc, char **argv) {

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}

	std::string configFilePath(argv[1]);

	// Parse the configuration file to get server configurations
    //ConfigParser configParser;
    std::vector<ServerConfig> serverConfigs;
	ServerConfig serverConfig("config/server.conf", "localhost", 8080);

	serverConfigs.push_back(serverConfig);

	// Create and initialize instances of HttpServer based on server configurations
    std::vector<HttpServer> httpServers;

	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		HttpServer httpServer;

        // Configure the HttpServer instance based on serverConfigs[i]
        httpServer.loadConfig(serverConfigs[i].getConfigFilePath());
        httpServer.setPort(serverConfigs[i].getPort());
		httpServer.setHost(serverConfigs[i].getHost());

        // Initialize and add to the vector of HttpServers
        if (httpServer.init()) {
            httpServers.push_back(httpServer);
        }
		else {
            std::cerr << "Error initializing HttpServer for server block " << (i + 1) << std::endl;
        }
	}

	HttpStatusCode::initStatusCodes();

	// Start all initialized servers
    for (size_t i = 0; i < httpServers.size(); ++i) {
        httpServers[i].run();
    }
}