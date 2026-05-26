#include "webserv.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"

#include <fstream>
#include <sstream>

Config::Config() : _filePath(DEFAULT_CONFIG_PATH) {}

Config::Config(const std::string& filePath) : _filePath(filePath) {}

Config::Config(const Config& other) {
	*this = other;
}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		_filePath = other._filePath;
		_servers  = other._servers;
	}

	return *this;
}

Config::~Config() {}

void Config::validateListens(ServerConfig& srv, std::set<std::pair<std::string, int> >& seen) {
	if (srv.listens.empty())
		throw std::runtime_error("server block has no listen directive");

	std::vector<std::pair<std::string, int> > listens;
	for (size_t i = 0; i < srv.listens.size(); ++i) {
		std::string raw = srv.listens[i].first;
		std::string host;
		std::string portStr;

		size_t colon = raw.find(':');
		if (colon != std::string::npos) {
			host = raw.substr(0, colon);
			portStr = raw.substr(colon + 1);
		} else {
			host = defaults::DEFAULT_HOST;
			portStr = raw;
		}
		
		if (host.empty())
			host = defaults::DEFAULT_HOST;

		int port = utils::parseNum<size_t>(portStr);
		if (port < 1 || port > 65535)
			throw std::runtime_error("invalid port for listen directive: " + portStr);

		listens.push_back(std::make_pair(host, port));
	}

	for (size_t i = 0; i < listens.size(); ++i) {
		const std::pair<std::string, int>& l = listens[i];

		if (seen.count(l))
			throw std::runtime_error("duplicate listen for " + l.first + ":" + utils::toString(l.second));

		seen.insert(l);
	}

	srv.listens = listens;
}

void Config::applyDefaults(ServerConfig& srv) {
	if (srv.root.empty())
		srv.root = defaults::DEFAULT_ROOT;
	if (srv.index.empty())
		srv.index = defaults::DEFAULT_INDEX;
	if (srv.clientMaxBodySize == 0)
		srv.clientMaxBodySize = defaults::CLIENT_MAX_BODY_SIZE;
	if (srv.clientMaxHeaderSize == 0)
		srv.clientMaxHeaderSize = defaults::CLIENT_MAX_HEADER_SIZE;
	if (srv.clientHeaderTimeout == 0)
		srv.clientHeaderTimeout = defaults::CLIENT_HEADER_TIMEOUT;
	if (srv.clientBodyTimeout == 0)
		srv.clientBodyTimeout = defaults::CLIENT_BODY_TIMEOUT;

	for (size_t i = 0; i < srv.locations.size(); ++i) {
		LocationConfig& loc = srv.locations[i];

		if (loc.root.empty())
			loc.root = srv.root;
		if (loc.index.empty())
			loc.index = srv.index;
		if (loc.methods.empty())
			loc.methods.push_back("GET");
	}
}

void Config::validateLocation(const LocationConfig& loc) {
	if (loc.redirect.first != 0) {
		int code = loc.redirect.first;

		if (code < 300 || code > 399)
			throw std::runtime_error("invalid redirect status code in location \"" + loc.path + "\": " + utils::toString(code));
	}

	for (std::map<int, std::string>::const_iterator it = loc.errorPages.begin();
		it != loc.errorPages.end(); ++it) {
		if (it->first < 100 || it->first > 599)
			throw std::runtime_error("invalid status code for error_page in location \"" + loc.path + "\": " + utils::toString(it->first));
	}
}

void Config::validateStatusCodes(const ServerConfig& srv) {
	for (std::map<int, std::string>::const_iterator it = srv.errorPages.begin();
		it != srv.errorPages.end(); ++it) {
		if (it->first < 100 || it->first > 599)
			throw std::runtime_error("invalid status code for error_page: " + utils::toString(it->first));
	}
}

void Config::validate(void) {
	std::set<std::pair<std::string, int> > seen;

	for (size_t i = 0; i < _servers.size(); ++i) {
		validateListens(_servers[i], seen);
		applyDefaults(_servers[i]);
		validateStatusCodes(_servers[i]);

		for (size_t j = 0; j < _servers[i].locations.size(); ++j)
			validateLocation(_servers[i].locations[j]);
	}
}

std::string Config::readFile(void) const {
	std::ifstream file(_filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("cannot open config file: " + _filePath);

	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

const std::vector<ServerConfig>& Config::getServers(void) const {
	return _servers;
}

void Config::debugPrintLocation(const LocationConfig& loc) const {
	Logger::debug("    Location {");
	Logger::debug("      path: " + loc.path);
	Logger::debug("      root: " + loc.root);
	Logger::debug("      index: " + loc.index);
	Logger::debug("      autoindex: " + (loc.autoindex ? std::string("on"): std::string("off")));
	Logger::debug("      upload_store: " + (!loc.uploadStore.empty() ? loc.uploadStore : "(empty)"));
	Logger::debug("      redirect: " + (loc.redirect.first != 0
		? utils::toString(loc.redirect.first) + " -> " + loc.redirect.second
		: "(empty)"));

	Logger::debug("      methods:");
	if (!loc.methods.empty()) {
		for (size_t j = 0; j < loc.methods.size(); ++j)
			Logger::debug("        " + loc.methods[j]);
	} else
		Logger::debug("        (empty)");

	Logger::debug("      cgi_extensions:");
	if (!loc.cgiExtensions.empty()) {
		for (size_t i = 0; i < loc.cgiExtensions.size(); ++i)
			Logger::debug("        " + loc.cgiExtensions[i]);
	} else
		Logger::debug("        (empty)");

	Logger::debug("      error_pages:");
	if (!loc.errorPages.empty()) {
		std::map<int, std::string>::const_iterator it;
		for (it = loc.errorPages.begin(); it != loc.errorPages.end(); ++it)
			Logger::debug("        " + utils::toString(it->first) + " -> " + it->second);
	} else
		Logger::debug("        (empty)");

	Logger::debug("    }");
}

void Config::debugPrintServer(const ServerConfig& srv) const {
	Logger::debug("Server {");

	Logger::debug("  listens");
	for (size_t j = 0; j < srv.listens.size(); ++j)
		Logger::debug("    " + srv.listens[j].first + ":" + utils::toString(srv.listens[j].second));

	Logger::debug("  root: " + srv.root);
	Logger::debug("  index: " + srv.index);
	Logger::debug("  autoindex: " + (srv.autoindex ? std::string("on") : std::string("off")));

	Logger::debug("  methods:");
	if (!srv.methods.empty()) {
		for (size_t j = 0; j < srv.methods.size(); ++j)
			Logger::debug("    " + srv.methods[j]);
	} else
		Logger::debug("    (empty)");

	Logger::debug("  cgi_extensions:");
	if (!srv.cgiExtensions.empty()) {
		for (size_t j = 0; j < srv.cgiExtensions.size(); ++j)
			Logger::debug("    " + srv.cgiExtensions[j]);
	} else
		Logger::debug("    (empty)");

	Logger::debug("  client_max_header_size: " + utils::toString(srv.clientMaxHeaderSize));
	Logger::debug("  client_max_body_size: " + utils::toString(srv.clientMaxBodySize));
	Logger::debug("  client_header_timeout: " + utils::toString(srv.clientHeaderTimeout));
	Logger::debug("  client_body_timeout: " + utils::toString(srv.clientBodyTimeout));

	Logger::debug("  error_pages:");
	if (!srv.errorPages.empty()) {
		std::map<int, std::string>::const_iterator it;
		for (it = srv.errorPages.begin(); it != srv.errorPages.end(); ++it)
			Logger::debug("    " + utils::toString(it->first) + " -> " + it->second);
	}
	else
		Logger::debug("    (empty)");

	Logger::debug("  locations:");
	for (size_t j = 0; j < srv.locations.size(); ++j)
		debugPrintLocation(srv.locations[j]);

	Logger::debug("}");
}

void Config::debugPrint(void) const {
	for (size_t i = 0; i < _servers.size(); ++i)
		debugPrintServer(_servers[i]);
}

void Config::load() {
	std::string source = readFile();

	ConfigParser cp(source);
	_servers = cp.parse();

	validate();

	debugPrint();
}
