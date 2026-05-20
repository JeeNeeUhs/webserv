#include "webserv.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

Config::Config() : _filePath(DEFAULT_CONFIG_PATH) {}

Config::Config(const std::string& filePath) : _filePath(filePath) {}

Config::Config(const Config& other) {
	operator=(other);
}

// TODO: add other members
Config& Config::operator=(const Config& other) {
	if (this != &other) {
		this->_servers = other._servers;
	}

	return *this;
}

Config::~Config() {}

#include <fstream>
#include <string>

void Config::expectToken(const std::string& expected) {
	if (_currToken != expected)
		throw std::runtime_error("expected '" + expected + "', but got '" + _currToken + "'");

	_currToken = getNextToken();
}

std::string Config::getNextToken() {
	std::string token;
	char c;
	
	while (_file.get(c)) {
		if (c == '#') {
			std::string dummy;
			std::getline(_file, dummy);
			continue;
		}

		if (std::isspace(c)) {
			if (!token.empty())
				return token;
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!token.empty()) {
				_file.unget();
				return token;
			}

			token += c;
			return token;
		}

		token += c;
	}

	return token;
}

void Config::parseLocation(Server& srv, const std::string& parentPath, Location currentState) {
	std::string locPath = _currToken;
	
	if (!parentPath.empty()) {
		if (locPath.find(parentPath) != 0)
			throw std::runtime_error("location \"" + locPath + "\" is outside location \"" + parentPath + "\"");
	}
	currentState.path = locPath;

	_currToken = getNextToken();
	expectToken("{");

	while (!_currToken.empty() && _currToken != "}") {
		std::string directive = _currToken;
		_currToken = getNextToken();

		if (directive == "root")
			currentState.root = _currToken;
		else if (directive == "index")
			currentState.index = _currToken;
		else if (directive == "methods") {
			// currentState.methods.clear();
			while (_currToken != ";") {
				currentState.methods.push_back(_currToken);
				_currToken = getNextToken();
			}

			expectToken(";");
			continue;
		}
		else if (directive == "location") {
			parseLocation(srv, currentState.path, currentState);
			continue;
		}
		else
			throw std::runtime_error("unknown directive in location block: " + directive);

		_currToken = getNextToken();
		expectToken(";");
	}

	expectToken("}");
	srv.locations.push_back(currentState);
}

void Config::parseServer(void) {
	_currToken = getNextToken(); // skip 'server' directive
	expectToken("{");

	Server srv;

	Location baseLoc;
	baseLoc.path = "/";

	while (!_currToken.empty() && _currToken != "}") {
		std::string directive = _currToken;
		_currToken = getNextToken();

		if (directive == "listen")
			srv.addListen(_currToken);
		else if (directive == "client_max_header_size")
			srv.clientMaxHeaderSize = parseInt(_currToken);
		else if (directive == "client_max_body_size")
			srv.clientMaxBodySize = parseInt(_currToken);
		else if (directive == "client_header_timeout")
			srv.clientHeaderTimeout = parseInt(_currToken);
		else if (directive == "client_body_timeout")
			srv.clientBodyTimeout = parseInt(_currToken);
		else if (directive == "root") {
			srv.root = _currToken;
			baseLoc.root = _currToken;
		}
		else if (directive == "index")
			baseLoc.index = _currToken;
		else if (directive == "cgi_extension")
			srv.cgi_extensions.push_back(_currToken);
		else if (directive == "error_page") {
			if (_currToken.size() != 3
				&& std::string("12345").find(_currToken[0]) == std::string::npos)
				throw std::runtime_error("invalid error code for error_page directive");

			int errorCode = parseInt(_currToken);
			std::string errorPage = getNextToken();

			srv.error_pages[errorCode] = errorPage;
		}
		else if (directive == "methods") {
			while (_currToken != ";") {
				srv.methods.push_back(_currToken);
				_currToken = getNextToken();
			}
			
			// continue loop for this directive
			expectToken(";");
			continue;
		}
		else if (directive == "location") {
			parseLocation(srv, "", baseLoc);
			continue;
		}
		else
			throw std::runtime_error("unknown directive in server block: " + directive);

		_currToken = getNextToken();
		expectToken(";");
	}

	expectToken("}");

	// check for double base location "/"
	bool hasBaseLoc = false;
	for (size_t i = 0; i < srv.locations.size(); ++i) {
		if (srv.locations[i].path == "/") {
			hasBaseLoc = true;
			break;
		}
	}

	if (!hasBaseLoc)
		srv.locations.push_back(baseLoc);

	_servers.push_back(srv);
}

void Config::parseFile(void) {
	_file.open(_filePath);
	if (!_file.is_open())
		throw std::runtime_error("cannot open config file: " + _filePath);

	_currToken = getNextToken();

	while (!_currToken.empty()) {
		if (_currToken != "server")
			throw std::runtime_error("unexpected token outside server block: " + _currToken);
		parseServer();
	}

	// std::vector<Server>::iterator it;
	// for (it = _servers.begin(); it != _servers.end(); ++it) {
	// 	std::vector<std::pair<std::string, int> > vec = (*it).listens;

	// 	std::vector<std::pair<std::string, int> >::iterator it2;
	// 	for (it2 = vec.begin(); it2 != vec.end(); ++it2)
	// 		std::cout << (*it2).first << ", " << (*it2).second << std::endl;

	// 	std::cout << (*it).clientMaxHeaderSize << std::endl;
	// 	std::cout << (*it).clientMaxBodySize << std::endl;
	// 	std::cout << (*it).clientHeaderTimeout << std::endl;
	// 	std::cout << (*it).clientBodyTimeout << std::endl;
	// }

	_file.close();
}

std::vector<Server> Config::getServers(void) {
	return _servers;
}
