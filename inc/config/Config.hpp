#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "ConfigTypes.hpp"

# include <vector>
# include <set>

class Config {
	private:
		std::vector<ServerConfig> _servers;
		std::string _filePath;

		std::string readFile(void) const;

		void validate(void);
		void validateListens(ServerConfig& srv, std::set<std::pair<std::string, int> >& seen);
		void validateLocation(const LocationConfig& loc);
		void validateStatusCodes(const ServerConfig& srv);
		void applyDefaults(ServerConfig& srv);

		void debugPrintLocation(const LocationConfig& loc) const;
		void debugPrintServer(const ServerConfig& srv) const;

	public:
		Config();
		Config(const std::string& filePath);
		Config(const Config& other);
		Config& operator=(const Config& other);
		~Config();

		const std::vector<ServerConfig>& getServers(void) const;

		void load(void);

		void debugPrint(void) const;
};

#endif
