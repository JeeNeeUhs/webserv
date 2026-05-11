#include "Server.hpp"

Server::Server() : sockFd(-1), client_max_body_size(0), root(""), index(""), autoindex(false) {}

Server::Server(const Server& other)
	: sockFd(other.sockFd),
	listens(other.listens),
	client_max_body_size(other.client_max_body_size),
	root(other.root),
	index(other.index),
	autoindex(other.autoindex),
	cgi_extensions(other.cgi_extensions),
	error_pages(other.error_pages),
	methods(other.methods),
	locations(other.locations) {}

Server& Server::operator=(const Server& other) {
	if (this != &other) {
		sockFd = other.sockFd;
		listens = other.listens;
		client_max_body_size = other.client_max_body_size;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		cgi_extensions = other.cgi_extensions;
		error_pages = other.error_pages;
		methods = other.methods;
		locations = other.locations;
	}

	return *this;
}

Server::~Server() {}

int Server::getSockFd() const {
	return sockFd;
}

const std::vector<std::pair<std::string, int> >& Server::getListens() const {
	return listens;
}

const std::string& Server::getRoot() const {
	return root;
}

const std::string& Server::getIndex() const {
	return index;
}

std::vector<Location>& Server::getLocations() {
	return locations;
}

void Server::fill() {
	listens.push_back(std::make_pair(std::string("0.0.0.0"), 8080));
	listens.push_back(std::make_pair(std::string("127.0.0.1"), 9090));
	
	client_max_body_size = 1024 * 1024;
	root = "/Users/nothing/cowd";
	index = "index.html";
	autoindex = false;
	cgi_extensions.push_back(".py");
	methods.push_back("GET");
	
	Location loc1;
	Location loc2;
	loc1.fill(this);
	loc2.fill2(this);
	locations.push_back(loc1);
	locations.push_back(loc2);
}
