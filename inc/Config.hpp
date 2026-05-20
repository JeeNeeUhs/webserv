#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"

# include <string>
# include <vector>
# include <map> 
# include <fstream>

class Config {
	private:
		std::vector<Server> _servers;

		std::ifstream	_file;
		std::string		_filePath;
		std::string		_currToken;

		std::string getNextToken(void);
		void expectToken(const std::string& expected);

		void parseLocation(Server& srv, const std::string& parentPath, Location currentState);
		void parseServer(void);

	public:
		Config();
		Config(const std::string& filePath);
		Config(const Config& other);
		Config& operator=(const Config& other);
		~Config();

		void parseFile(void);

		std::vector<Server> getServers(void);
};

#endif
