#include "core/HttpServer.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "core/HttpStatusCode.hpp"

#include <vector>

int main(int argc, char **argv) {

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}

	try
	{
		ConfigParser parser(argv[1]);
		parser.proccess_input();
		std::cout << parser.getServerConfigVector().back()->getPorts()[0] << std::endl;
		std::cout << parser.getServerConfigVector().back()->getPorts()[1] << std::endl;

		std::cout << parser.getServerConfigVector().back()->getServer_names()[0] << std::endl;
		std::cout << parser.getServerConfigVector().back()->getServer_names()[1] << std::endl;

		std::cout << parser.getServerConfigVector().back()->getRoot() << std::endl;


		std::cout << parser.getServerConfigVector().back()->getError_codes()[0] << std::endl;
		std::cout << parser.getServerConfigVector().back()->getError_pages()[0] << std::endl;

		std::cout << parser.getServerConfigVector().back()->getClient_max_body_size() << std::endl;

		std::cout << parser.getServerConfigVector().back()->getLocations()[0]->getPath() << std::endl;
		std::cout << parser.getServerConfigVector().back()->getLocations()[0]->getRoot() << std::endl;

		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getMethods()[0] << std::endl;
		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getMethods()[1] << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
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

		for (size_t i = 0; i < serverConfig.getLocations().size(); i++) {
			Location location = *(serverConfig.getLocations()[i]);
			RouteHandler routeHandler(location);
			httpServer.addRouteHandler(routeHandler);
		}

        // Configure the HttpServer instance based on serverConfigs[i]
        httpServer.loadConfig(serverConfigs[i].getConfigFilePath());
        httpServer.setPort(serverConfigs[i].getPort());//alterei os ports de int para vector de ints (pode haver varios)
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