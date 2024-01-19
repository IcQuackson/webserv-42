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
	ServerConfig *serverConfig = new ServerConfig();

	if (argc == 2) {
		ConfigParser parser(argv[1]);

		try
		{	
			if(!parser.proccess_input())
				return (0);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (0);
		}
		
		//std::string configFilePath(argv[1]);
		serverConfigs = parser.getServerConfigVector();
	}
	else {

		ServerConfig::setDefaultServer(serverConfig);
		serverConfigs.push_back(serverConfig);
	}


	//serverConfig = *parser.getServerConfigVector()[0];

	// Create and initialize instances of HttpServer based on server configurations
    std::vector<HttpServer> httpServers = std::vector<HttpServer>();

	for (size_t i = 0; i < serverConfigs.size(); i++) {
		HttpServer httpServer(*serverConfigs[i]);

		for (size_t j = 0; j < serverConfigs[i]->getLocations().size(); j++) {
			std::cout << serverConfigs[i]->getLocations().size() << std::endl;
			Location *location = serverConfigs[i]->getLocations()[j];
			RouteHandler routeHandler(*serverConfigs[i],*location);
			//delete location;
			httpServer.addRouteHandler(routeHandler);
		}

        // Configure the HttpServer instance based on serverConfigs[i]
        httpServer.loadConfig(serverConfigs[i]->getConfigFilePath());
        httpServer.setPort(serverConfigs[i]->getPort());
		httpServer.setHost(serverConfigs[i]->getHost());

        // Initialize and add to the vector of HttpServers
        httpServers.push_back(httpServer);
	}

	std::map<std::string, RouteHandler> routes = httpServers.at(0).getRouteHandlers();
	//RouteHandler handler = routes["/resources"];

	std::map<std::string, RouteHandler>::iterator it;
	for (it = routes.begin(); it != routes.end(); ++it) {
	}
	//std::cout << "debug: " << handler.getLocation() << std::endl;

	HttpStatusCode::initStatusCodes();
	HttpServer::setupServers(httpServers);
	HttpServer::runServers(httpServers);
	// Start all initialized servers
}