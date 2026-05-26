#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include "ConfigTypes.hpp"
# include "Listener.hpp"
# include "Connection.hpp"

# include <poll.h>
# include <vector>

typedef struct pollfd pollfd_t;

class ServerManager {
	private:
		std::vector<ServerConfig> _configs;

		std::vector<pollfd_t>		_pollFds;
		std::map<int, Listener>		_listeners;
		std::map<int, Connection>	_connections;

		void checkTimeouts(void);
		void acceptClients(int listenFd);
		bool handleClient(pollfd_t& pfd, short revents);
		bool sendToClient(int fd, Connection& c);

		bool processBuffer(pollfd_t& pfd, Connection& c);
		bool setErrorResponse(pollfd_t& pfd, Connection& c);
		bool readFromClient(pollfd_t& pfd, Connection& c);

		void addPollFd(int fd, short events);
		void clearPollSet(void);

	public:
		ServerManager();
		ServerManager(const std::vector<ServerConfig>& configs);
		ServerManager(const ServerManager& other);
		ServerManager& operator=(const ServerManager& other);
		~ServerManager();

		void setup(void);
		void run(void);
};

#endif
