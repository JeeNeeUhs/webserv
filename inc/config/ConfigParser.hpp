#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

# include "ConfigTypes.hpp"

class ConfigParser {
	private:
		std::string	_src;
		size_t		_line;
		std::string	_curr;
		size_t		_pos;

		std::string getNextToken(void);
		void expect(const std::string& token);

		ServerConfig parseServer(void);
		void	parseLocation(ServerConfig& srv, const std::string& parentPath, LocationConfig inherited);
		void	parseServerDirective(ServerConfig& srv, LocationConfig& baseLoc, const std::string& directive);
		void	parseLocationDirective(LocationConfig& loc, const std::string& directive, std::vector<std::string>& seen);

		std::vector<std::string> consumeMethodList();
		void error(const std::string& msg) const;

	public:
		ConfigParser();
		ConfigParser(const std::string& src);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser();

		std::vector<ServerConfig> parse(void);
};

#endif
