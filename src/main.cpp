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

	ConfigParser parser(argv[1]);

	try
	{	
		if(!parser.proccess_input())
			return (0);

		/* std::cout << parser.getServerConfigVector().back()->getPort() << std::endl;
		std::cout << parser.getServerConfigVector().back()->getPort() << std::endl;
		std::cout << parser.getServerConfigVector()[0]->getPort() << std::endl;
		std::cout << parser.getServerConfigVector()[0]->getHost() << std::endl;

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

		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getRedirection() << std::endl;
		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getDirectoryListing() << std::endl;

		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getDefaultFile() << std::endl;

		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getCgiPath() << std::endl;
		std::cout << parser.getServerConfigVector().back()->getLocations().back()->getCgiExtension() << std::endl;
 */
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (0);
	}
	
	std::string configFilePath(argv[1]);

	// Parse the configuration file to get server configurations
    //ConfigParser configParser;
    std::vector<ServerConfig> serverConfigs;
	ServerConfig serverConfig;

	//ServerConfig::setDefaultServer(serverConfig);

	serverConfig = *parser.getServerConfigVector()[0];
	serverConfigs.push_back(serverConfig);

	// Create and initialize instances of HttpServer based on server configurations
    std::vector<HttpServer> httpServers;

	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		HttpServer httpServer;

		for (size_t i = 0; i < serverConfig.getLocations().size(); i++) {
			Location location = *(serverConfig.getLocations()[i]);
			std::cout << "setup location: " << location << std::endl;
			RouteHandler routeHandler(location);
			httpServer.addRouteHandler(routeHandler);
		}

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

	std::map<std::string, RouteHandler> routes = httpServers.at(0).getRouteHandlers();
	//RouteHandler handler = routes["/resources"];

	std::map<std::string, RouteHandler>::iterator it;
	for (it = routes.begin(); it != routes.end(); ++it) {
		std::cout << "Key: " << it->first << ", Value: " << std::endl << it->second.getLocation() << std::endl;
	}
	//std::cout << "debug: " << handler.getLocation() << std::endl;

	HttpStatusCode::initStatusCodes();

	// Start all initialized servers
    for (size_t i = 0; i < httpServers.size(); ++i) {
        httpServers[i].run();
    }
}