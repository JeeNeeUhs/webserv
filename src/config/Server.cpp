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
