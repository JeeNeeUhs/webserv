#ifndef CONFIG_TYPES_HPP
# define CONFIG_TYPES_HPP

# include "webserv.hpp"

# include <string>
# include <vector>
# include <map>
# include <ctime>

struct LocationConfig {
	std::string	path;
	std::string	root;
	std::string	index;
	bool		autoindex;

	std::pair<int, std::string>	redirect;
	std::string					uploadStore;

	std::vector<std::string>	cgiExtensions;
	std::map<int, std::string>	errorPages;
	std::vector<std::string>	methods;

	LocationConfig()
		: autoindex(false), redirect(0, "") {}
};

struct ServerConfig {
	std::vector<std::pair<std::string, int> > listens;

	size_t clientMaxBodySize;
	time_t clientBodyTimeout;

	std::string	root;
	std::string	index;
	bool		autoindex;

	std::vector<std::string>	cgiExtensions;
	std::map<int, std::string>	errorPages;
	std::vector<std::string>	methods;

	std::vector<LocationConfig>	locations;

	ServerConfig()
		: clientMaxBodySize(defaults::CLIENT_MAX_BODY_SIZE),
		clientBodyTimeout(defaults::CLIENT_BODY_TIMEOUT),
		root(defaults::DEFAULT_ROOT),
		index(defaults::DEFAULT_INDEX),
		autoindex(false) {}
};

#endif