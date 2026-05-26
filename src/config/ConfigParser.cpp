#include "ConfigParser.hpp"
#include "utils.hpp"

#include <sstream>
#include <algorithm>

ConfigParser::ConfigParser()
	: _src(""), _line(1), _curr(""), _pos(0) {}

ConfigParser::ConfigParser(const std::string& src)
	: _src(src), _line(1), _curr(""), _pos(0) {}

ConfigParser::ConfigParser(const ConfigParser& other) {
	operator=(other);
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
	if (this != &other) {
		_src  = other._src;
		_pos  = other._pos;
		_line = other._line;
		_curr = other._curr;
	}

	return *this;
}

ConfigParser::~ConfigParser() {}

void ConfigParser::error(const std::string& msg) const {
	throw std::runtime_error("config error in line " + utils::toString(_line) + ": " + msg);
}


std::string ConfigParser::getNextToken() {
	std::string token;

	while (_pos < _src.size()) {
		char c = _src[_pos];

		if (c == '#') {
			while (_pos < _src.size() && _src[_pos] != '\n')
				++_pos;

			continue;
		}

		if (std::isspace(c)) {
			if (c == '\n')
				++_line;
			
			++_pos;

			if (!token.empty())
				return token;

			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!token.empty())
				return token;

			++_pos;
			token += c;

			return token;
		}

		token += c;
		++_pos;
	}

	return token;
}

void ConfigParser::expect(const std::string& token) {
	if (_curr != token)
		error("expected '" + token + "', but got '" + _curr + "'");

	_curr = getNextToken();
}

std::vector<std::string> ConfigParser::consumeMethodList() {
	std::vector<std::string> methods;

	while (!_curr.empty() && _curr != ";") {
		if (_curr != "GET" && _curr != "POST" && _curr != "DELETE")
			error("invalid value for methods directive: " + _curr);
		if (std::find(methods.begin(), methods.end(), _curr) != methods.end())
			error("duplicate method for methods directive: " + _curr);

		methods.push_back(_curr);
		_curr = getNextToken();
	}

	if (methods.empty())
		error("methods directive expects at least one method");

	return methods;
}

void ConfigParser::parseLocationDirective(LocationConfig& loc, const std::string& directive, std::vector<std::string>& seen) {
	if (directive == "root") {
		if (std::find(seen.begin(), seen.end(), "root") != seen.end())
			error("duplicate root directive in location context");

		loc.root = _curr;
		seen.push_back("root");
	}
	else if (directive == "index") {
		if (std::find(seen.begin(), seen.end(), "index") != seen.end())
			error("duplicate index directive in location context");

		loc.index = _curr;
		seen.push_back("index");
	}
	else if (directive == "autoindex") {
		if (std::find(seen.begin(), seen.end(), "autoindex") != seen.end())
			error("duplicate autoindex directive in location context");

		if (_curr != "on" && _curr != "off")
			error("invalid value for autoindex directive: " + _curr);
		loc.autoindex = (_curr == "on");
		seen.push_back("autoindex");
	}
	else if (directive == "redirect") {
		if (std::find(seen.begin(), seen.end(), "redirect") != seen.end())
			error("duplicate redirect directive in location context");

		int code = utils::parseNum<size_t>(_curr);
		std::string target = getNextToken();
		loc.redirect = std::make_pair(code, target);
		seen.push_back("redirect");
	}
	else if (directive == "upload_store") {
		if (std::find(seen.begin(), seen.end(), "upload_store") != seen.end())
			error("duplicate upload_store directive in location context");

		loc.uploadStore = _curr;
		seen.push_back("upload_store");
	}
	else if (directive == "cgi_extension") {
		if (std::find(loc.cgiExtensions.begin(), loc.cgiExtensions.end(), _curr) != loc.cgiExtensions.end())
			error("duplicate extension for cgi_extension directive: " + _curr);

		loc.cgiExtensions.push_back(_curr);
	}
	else if (directive == "error_page") {
		int code = utils::parseNum<size_t>(_curr);
		std::string page = getNextToken();
		loc.errorPages[code] = page;
	}
	else if (directive == "methods") {
		if (std::find(seen.begin(), seen.end(), "methods") != seen.end())
			error("duplicate methods directive in location context");

		loc.methods = consumeMethodList(); // overrides inherited methods
		seen.push_back("methods");
		expect(";");
		return;
	}
	else
		error("unknown directive in location block: " + directive);

	_curr = getNextToken();
	expect(";");
}

void ConfigParser::parseServerDirective(ServerConfig& srv, LocationConfig& baseLoc, const std::string& directive) {
	if (directive == "listen")
		// host/port splitting will be in Config::validate 
		srv.listens.push_back(std::make_pair(_curr, 0));
	else if (directive == "client_max_header_size")
		srv.clientMaxHeaderSize = utils::parseNum<size_t>(_curr);
	else if (directive == "client_max_body_size")
		srv.clientMaxBodySize = utils::parseNum<size_t>(_curr);
	else if (directive == "client_header_timeout")
		srv.clientHeaderTimeout = utils::parseNum<time_t>(_curr);
	else if (directive == "client_body_timeout")
		srv.clientBodyTimeout = utils::parseNum<time_t>(_curr);
	else if (directive == "root") {
		srv.root = _curr;
		baseLoc.root = _curr;
	}
	else if (directive == "index") {
		srv.index = _curr;
		baseLoc.index = _curr;
	}
	else if (directive == "autoindex") {
		if (_curr != "on" && _curr != "off")
			error("invalid value for autoindex directive: " + _curr);
		srv.autoindex = (_curr == "on");
		baseLoc.autoindex = srv.autoindex;
	}
	else if (directive == "cgi_extension") {
		srv.cgiExtensions.push_back(_curr);
		baseLoc.cgiExtensions.push_back(_curr);
	}
	else if (directive == "error_page") {
		int code = utils::parseNum<size_t>(_curr);
		std::string page = getNextToken();
		srv.errorPages[code] = page;
		baseLoc.errorPages[code] = page;
	}
	else if (directive == "methods") {
		std::vector<std::string> methods = consumeMethodList();
		srv.methods = methods;
		baseLoc.methods = methods;
		expect(";");
		return;
	}
	else
		error("unknown directive in server block: " + directive);

	_curr = getNextToken();
	expect(";");
}

void ConfigParser::parseLocation(ServerConfig& srv, const std::string& parentPath, LocationConfig inherited) {
	std::string locPath = _curr;

	if (!parentPath.empty() && locPath.find(parentPath) != 0)
		error("location \"" + locPath + "\" is outside location \"" + parentPath + "\"");

	inherited.path = locPath;
	// nested location shouldnt inherit parent redirect.
	inherited.redirect = std::make_pair(0, std::string(""));

	_curr = getNextToken();
	expect("{");

	std::vector<std::string> seen;

	while (!_curr.empty() && _curr != "}") {
		std::string directive = _curr;
		_curr = getNextToken();

		if (directive == "location") {
			parseLocation(srv, inherited.path, inherited);
			continue;
		}

		parseLocationDirective(inherited, directive, seen);
	}

	expect("}");
	srv.locations.push_back(inherited);
}

ServerConfig ConfigParser::parseServer() {
	_curr = getNextToken(); // skip the 'server' directive
	expect("{");

	ServerConfig srv;

	LocationConfig baseLoc;
	baseLoc.path = "/";

	while (!_curr.empty() && _curr != "}") {
		std::string directive = _curr;
		_curr = getNextToken();

		if (directive == "location") {
			parseLocation(srv, "", baseLoc);
			continue;
		}

		parseServerDirective(srv, baseLoc, directive);
	}

	expect("}");

	// check if there is only one base location
	bool hasBaseLoc = false;
	for (size_t i = 0; i < srv.locations.size(); ++i) {
		if (srv.locations[i].path == "/") {
			hasBaseLoc = true;
			break;
		}
	}
	if (!hasBaseLoc)
		srv.locations.push_back(baseLoc);

	return srv;
}

std::vector<ServerConfig> ConfigParser::parse() {
	std::vector<ServerConfig> servers;

	_curr = getNextToken();

	while (!_curr.empty()) {
		if (_curr != "server")
			error("expected 'server' block, but got '" + _curr + "'");
		servers.push_back(parseServer());
	}

	if (servers.empty())
		throw std::runtime_error("config error: no server blocks defined");

	return servers;
}
