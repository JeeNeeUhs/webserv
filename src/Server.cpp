#include "Server.hpp"
#include "utils.hpp"

#include <stdexcept>

Server::Server()
	: sockFd(-1),
	clientMaxHeaderSize(0),
	clientMaxBodySize(0),
	clientHeaderTimeout(0),
	clientBodyTimeout(0),
	root(""),
	index(""),
	autoindex(false) {}

Server::Server(const Server& other) {
	operator=(other);
}

Server& Server::operator=(const Server& other) {
	if (this != &other) {
		sockFd = other.sockFd;
		listens = other.listens;
		clientMaxBodySize = other.clientMaxBodySize;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		cgiExtensions = other.cgiExtensions;
		errorPages = other.errorPages;
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

void Server::addListen(const std::string& listenVal) {
	size_t colonPos = listenVal.find(":");

	if (colonPos != std::string::npos) {
		std::string host = listenVal.substr(0, colonPos);
		std::string port = listenVal.substr(colonPos + 1);

		int parsedPort = parseInt(port);
		if (parsedPort < 1 || parsedPort > 65535)
			throw std::runtime_error("invalid port for listen directive: " + port);

		listens.push_back(std::make_pair(host, parsedPort));
	} else {
		int parsedPort = parseInt(listenVal);
		if (parsedPort < 1 || parsedPort > 65535)
			throw std::runtime_error("invalid port for listen directive: " + listenVal);

		listens.push_back(std::make_pair("0.0.0.0", parsedPort));
	}
}

void Server::fill() {
	listens.push_back(std::make_pair(std::string("0.0.0.0"), 8080));
	listens.push_back(std::make_pair(std::string("127.0.0.1"), 9090));
	
	clientMaxBodySize = 1024 * 1024;
	root = "/Users/nothing/cowd";
	index = "index.html";
	autoindex = false;
	cgiExtensions.push_back(".py");
	methods.push_back("GET");
	
	Location loc1;
	Location loc2;
	loc1.fill(this);
	loc2.fill2(this);
	locations.push_back(loc1);
	locations.push_back(loc2);
}
