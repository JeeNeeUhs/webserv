#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include "Location.hpp"

class Server {
	private:
		int sockFd;

		std::string host;
		int port;
		size_t client_max_body_size;
		std::string root;
		std::string index;
		bool autoindex;

		std::vector<std::string> cgi_extensions;
		std::vector<t_error_page> error_pages;
		std::vector<std::string> methods;
		std::vector<Location> locations;

	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();

		int getSockFd() const;
		std::string getHost() const;
		int getPort() const;
		size_t getClientMaxBodySize() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoindex() const;
		std::vector<std::string> getCgiExtensions() const;
		std::vector<t_error_page> getErrorPages() const;
		std::vector<std::string> getMethods() const;
		std::vector<Location>& getLocations();

		void fill(); // test purposes

};

#endif
