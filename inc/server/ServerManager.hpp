#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include "ConfigTypes.hpp"
# include "Listener.hpp"
# include "Connection.hpp"
# include "SessionHandler.hpp"

# include <poll.h>
# include <vector>

typedef struct pollfd pollfd_t;

class ServerManager {
	private:
		std::vector<ServerConfig> _servers;

		std::vector<pollfd_t>		_pollFds;
		std::map<int, Listener>		_listeners;
		std::map<int, Connection>	_connections;
		std::map<int, Connection*>	_cgiReadFds; //cgi read fd to connection fd
		std::map<int, Connection*>	_cgiWriteFds; //cgi write fd to connection fd

		SessionHandler _sessionHandler;

		void checkTimeouts(void);
		void acceptClients(int listenFd);
		bool handleClient(pollfd_t& pfd, short revents);
		bool sendToClient(int fd, Connection& c);

		bool setErrorResponse(pollfd_t& pfd, Connection& c, size_t code);
		bool processBuffer(pollfd_t& pfd, Connection& c);
		bool readFromClient(pollfd_t& pfd, Connection& c);

		void addPollFd(int fd, short events);
		void clearPollSet(void);
		void closeConnection(int& fd);

		bool readFromCgi(pollfd_t& pfd, Connection& c);
		bool writeToCgi(pollfd_t& pfd, Connection& c);

	public:
		ServerManager();
		ServerManager(const std::vector<ServerConfig>& servers);
		ServerManager(const ServerManager& other);
		ServerManager& operator=(const ServerManager& other);
		~ServerManager();

		pollfd_t* getConnetionPfd(Connection& c);

		void setup(void);
		void run(void);
};

HTTPResponse buildErrorResponse(const ServerConfig& config, size_t statusCode);

#endif
