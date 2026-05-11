#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <string>
# include <vector>
# include <map>

class Server {
	private:
		int sockFd;
		std::vector<std::pair<std::string, int> > listens;

		size_t client_max_body_size;
		std::string root;
		std::string index;
		bool autoindex;

		std::vector<std::string> cgi_extensions;
		std::map<int, std::string> error_pages;
		std::vector<std::string> methods;
		std::vector<Location> locations;

	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();

		int getSockFd() const;
		const std::vector<std::pair<std::string, int> >& getListens() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		std::vector<Location>& getLocations();

		void fill(); // test purposes
};

#endif
