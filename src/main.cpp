#include "core/HttpServer.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "core/HttpStatusCode.hpp"

#include <vector>

int main(int argc, char **argv) {

	if (argc != 1 && argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path> or no argument" << std::endl;
		return 1;
	}

	std::vector<ServerConfig*> serverConfigs;
	if (argc == 2) {
		ConfigParser parser(argv[1]);

		try {
			if(!parser.proccess_input()) {
				return (0);
			}
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			return (0);
		}
		serverConfigs = parser.getServerConfigVector();
	}
	else {
		ServerConfig* serverConfig = new ServerConfig();
		ServerConfig::setDefaultServer(serverConfig);
		serverConfigs.push_back(serverConfig);
	}

	// Create and initialize instances of HttpServer based on server configurations
    std::vector<HttpServer*> httpServers = std::vector<HttpServer*>();

	for (size_t i = 0; i < serverConfigs.size(); i++) {
		HttpServer* httpServer = new HttpServer(*serverConfigs[i]);

		for (size_t j = 0; j < serverConfigs[i]->getLocations().size(); j++) {
			Location location = *serverConfigs[i]->getLocations()[j];
			RouteHandler routeHandler(location);
			httpServer->addRouteHandler(routeHandler);
		}

        // Configure the HttpServer instance based on serverConfigs[i]
        httpServer->loadConfig(serverConfigs[i]->getConfigFilePath());
        httpServer->setPort(serverConfigs[i]->getPort());
		httpServer->setHost(serverConfigs[i]->getHost());

        httpServers.push_back(httpServer);
	}

	HttpStatusCode::initStatusCodes();
	try {
		HttpServer::setupServers(httpServers);
		HttpServer::runServers(httpServers);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		for (size_t i = 0; i < httpServers.size(); i++) {
			delete httpServers[i];
		}
		for (size_t i = 0; i < serverConfigs.size(); i++) {
			ServerConfig::deleteServerConfig(serverConfigs[i]);
		}
	}
}