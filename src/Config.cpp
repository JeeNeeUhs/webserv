#include "webserv.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>

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

void Config::parseLocation(Server& srv, const std::string& parentPath, Location inheritedLoc) {
	std::set<std::string> setDirectives;

	std::string locPath = _currToken;

	if (!parentPath.empty()) {
		if (locPath.find(parentPath) != 0)
			throw std::runtime_error("location \"" + locPath + "\" is outside location \"" + parentPath + "\"");
	}
	inheritedLoc.path = locPath;

	_currToken = getNextToken();
	expectToken("{");

	while (!_currToken.empty() && _currToken != "}") {
		std::string directive = _currToken;
		_currToken = getNextToken();

		if (directive == "root") {
			if (setDirectives.count("root"))
				throw std::runtime_error("duplicate root directive in location context");

			inheritedLoc.root = _currToken;
			setDirectives.insert("root");
		}
		else if (directive == "index") {
			if (setDirectives.count("index"))
				throw std::runtime_error("duplicate index directive in location context");

			inheritedLoc.index = _currToken;
			setDirectives.insert("index");
		}
		else if (directive == "autoindex") {
			if (setDirectives.count("autoindex"))
				throw std::runtime_error("duplicate autoindex directive in location context");
			if (_currToken != "on" && _currToken != "off")
				throw std::runtime_error("invalid value for autoindex directive: " + _currToken);

			inheritedLoc.autoindex = _currToken == "on";
			setDirectives.insert("autoindex");
		}
		else if (directive == "redirect") {
			if (setDirectives.count("redirect"))
				throw std::runtime_error("duplicate redirect directive in location context");

			int redirectCode = parseInt(_currToken);
			std::string redirectPath = getNextToken();

			inheritedLoc.redirect = std::make_pair(redirectCode, redirectPath);
			setDirectives.insert("redirect");
		}
		else if (directive == "upload_store") {
			if (setDirectives.count("upload_store"))
				throw std::runtime_error("duplicate upload_store directive in location context");

			inheritedLoc.uploadStore = _currToken;
			setDirectives.insert("upload_store");
		}
		else if (directive == "cgi_extension") {
			inheritedLoc.cgiExtensions.push_back(_currToken);
		}
		else if (directive == "error_page") {
			if (_currToken.size() != 3
				&& std::string("12345").find(_currToken[0]) == std::string::npos)
				throw std::runtime_error("invalid value for error_page directive: " + _currToken);

			int errorCode = parseInt(_currToken);
			std::string errorPage = getNextToken();

			inheritedLoc.errorPages[errorCode] = errorPage;
		}
		else if (directive == "methods") {
			// clear the inherited methods vector
			inheritedLoc.methods.clear();

			while (_currToken != ";") {
				if (_currToken != "GET" && _currToken != "POST" && _currToken != "DELETE")
					throw std::runtime_error("invalid value for methods directive: " + _currToken);
				if (std::find(inheritedLoc.methods.begin(), inheritedLoc.methods.end(), _currToken)
					!= inheritedLoc.methods.end())
					throw std::runtime_error("duplicate method for methods directive: " + _currToken);

				inheritedLoc.methods.push_back(_currToken);
				_currToken = getNextToken();
			}

			expectToken(";");
			continue;
		}
		else if (directive == "location") {
			parseLocation(srv, inheritedLoc.path, inheritedLoc);
			continue;
		}
		else
			throw std::runtime_error("unknown directive in location block: " + directive);

		_currToken = getNextToken();
		expectToken(";");
	}

	expectToken("}");
	srv.locations.push_back(inheritedLoc);
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
		else if (directive == "cgi_extension") {
			srv.cgiExtensions.push_back(_currToken);
			baseLoc.cgiExtensions.push_back(_currToken);
		}
		else if (directive == "error_page") {
			if (_currToken.size() != 3
				&& std::string("12345").find(_currToken[0]) == std::string::npos)
				throw std::runtime_error("invalid value for error_page directive: " + _currToken);

			int errorCode = parseInt(_currToken);
			std::string errorPage = getNextToken();

			srv.errorPages[errorCode] = errorPage;
			baseLoc.errorPages[errorCode] = errorPage;
		}
		else if (directive == "methods") {
			while (_currToken != ";") {
				srv.methods.push_back(_currToken);
				baseLoc.methods.push_back(_currToken);

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
	_file.open(_filePath.c_str());
	if (!_file.is_open())
		throw std::runtime_error("cannot open config file: " + _filePath);

	_currToken = getNextToken();

	while (!_currToken.empty()) {
		if (_currToken != "server")
			throw std::runtime_error("unexpected token outside server block: " + _currToken);
		parseServer();
	}

	_file.close();
}

std::vector<Server> Config::getServers(void) {
	return _servers;
}
