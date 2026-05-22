#include "webserv.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <sstream>
#include <iostream>
#include <vector>

static std::string parseArgs(int argc, char *argv[]) {
	std::string filePath;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-d" || arg == "--debug") {
			Logger::setDebugMode(true);
			Logger::debug("debug mode activated, good luck :)");
		}
		else if (arg.length() > 0 && arg[0] == '-')
			throw std::invalid_argument("invalid option " + arg);
		else if (i == argc - 1)
			filePath = arg;
		else
			throw std::invalid_argument("too many arguments");
	}

	if (filePath.empty()) {
		Logger::warning("no config file provided, falling back to default: " + std::string(DEFAULT_CONFIG_PATH));
		filePath = DEFAULT_CONFIG_PATH;
	}

	return filePath;
}

static void printUsage(const std::string& reason) {
	std::cerr << PROGRAM_NAME << ": " << reason << "\n\n"
			<< "Usage: " << PROGRAM_NAME << " [OPTION]... [FILE]\n\n"
			<< "Options:\n  -d, --debug      enable debug output\n\n"
			<< "Arguments:\n  FILE             optional config file path\n";
}

int main(int argc, char *argv[]) {
	try {
		std::string configPath = parseArgs(argc, argv);

		Config config(configPath);
		config.load();

		// std::vector<Server> servers;
		// const std::vector<ServerConfig>& configs = config.getServers();
		// for (size_t i = 0; i < configs.size(); ++i)
			// servers.push_back(Server(configs[i]));

		// std::ostringstream msg;
		// msg << "loaded config with " << servers.size() << " server block(s)";
		// Logger::info(msg.str());

	} catch (const std::invalid_argument& e) {
		printUsage(e.what());
		return 1;
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return 1;
	}

	Logger::info("terminating...");
}
