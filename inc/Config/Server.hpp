#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include "Location.hpp"

class Server {
	private:
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

};

#endif
