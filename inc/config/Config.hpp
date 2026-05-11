#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>
# include <stdexcept>
# include "Location.hpp"
# include "Server.hpp"

typedef enum e_token_type {
	TOKEN_WORD,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_SEMICOLON,
	TOKEN_EOF
} t_token_type;

typedef struct s_token {
	t_token_type type;
	std::string  value;
} t_token;

class Config {
	private:
		std::vector<Server> servers;

		std::vector<t_token> tokenize(const std::string& content);
		void                 parseConfig(const std::vector<t_token>& tokens);
		Server               parseServerBlock(const std::vector<t_token>& tokens, size_t& i);
		Location             parseLocationBlock(const std::vector<t_token>& tokens, size_t& i);
		void                 parseServerDirective(const std::string& name, const std::vector<t_token>& tokens, size_t& i, Server& server);
		void                 parseLocationDirective(const std::string& name, const std::vector<t_token>& tokens, size_t& i, Location& loc);
		size_t               parseSize(const std::string& value);
		void                 validate();

	public:
		Config();
		Config(const std::string& filename);
		Config(const Config& other);
		Config& operator=(const Config& other);
		~Config();

		const std::vector<Server>& getServers() const;
};

#endif
