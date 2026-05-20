#include "webserv.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <iostream>
#include <vector>

std::string parseArgs(int argc, char *argv[]) {
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

int main(int argc, char *argv[]) {
	std::vector<Server> servers;

	try {
		std::string configPath = parseArgs(argc, argv);
		Config c(configPath);
		c.parseFile();

		servers = c.getServers();
	} catch (const std::invalid_argument& e) {
		std::cerr << PROGRAM_NAME << ": " << e.what() <<"\n\n"
				<< "Usage: " << PROGRAM_NAME << " [OPTION]... [FILE]\n\n"
				<< "Options:\n  -d, --debug      enable debug output\n\n"
				<< "Arguments:\n  FILE             optional config file path\n";
		return 1;
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return 1;
	}

	Logger::info("terminating...");
}
