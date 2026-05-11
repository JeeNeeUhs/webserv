#include "Server.hpp"

Server::Server() : host(""), port(0), client_max_body_size(0), root(""), index(""), autoindex(false) {}

Server::Server(const Server& other) : host(other.host), port(other.port), client_max_body_size(other.client_max_body_size), root(other.root), index(other.index), autoindex(other.autoindex), cgi_extensions(other.cgi_extensions), error_pages(other.error_pages), methods(other.methods), locations(other.locations) {}

Server& Server::operator=(const Server& other) {
	if (this != &other) {
		host = other.host;
		port = other.port;
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

std::string Server::getHost() const {
	return host;
}

int Server::getPort() const {
	return port;
}

size_t Server::getClientMaxBodySize() const {
	return client_max_body_size;
}

std::string Server::getRoot() const {
	return root;
}

std::string Server::getIndex() const {
	return index;
}

bool Server::getAutoindex() const {
	return autoindex;
}

std::vector<std::string> Server::getCgiExtensions() const {
	return cgi_extensions;
}

std::vector<t_error_page> Server::getErrorPages() const {
	return error_pages;
}

std::vector<std::string> Server::getMethods() const {
	return methods;
}

std::vector<Location>& Server::getLocations() {
	return locations;
}

void Server::fill() {
	host = "0.0.0.0";
	port = 8080;
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
