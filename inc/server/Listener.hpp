#ifndef LISTENER_HPP
# define LISTENER_HPP

# include "ConfigTypes.hpp"

# include <vector>

class Listener {
	private:
		const ServerConfig* _config;

		int	_fd;

		std::string	_host;
		int			_port;

	public:
		Listener();
		Listener(const std::string& host, int port);
		Listener(const Listener& other);
		Listener& operator=(const Listener& other);
		~Listener();

		const ServerConfig* getConfig() const;
		int getFd() const;
		const std::string& getHost() const;
		int getPort() const;

		void open(void);
		void close(void);
		void setConfig(const ServerConfig* cfg);
};

#endif
