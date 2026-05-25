#include "Listener.hpp"
#include "utils.hpp"

#include <sstream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Listener::Listener()
	: _fd(-1), _host(""), _port(0) {}

Listener::Listener(const std::string& host, int port)
	: _fd(-1), _host(host), _port(port) {}

Listener::Listener(const Listener& other) {
	operator=(other);
}

Listener& Listener::operator=(const Listener& other) {
	if (this != &other) {
		_fd = other._fd;
		_host = other._host;
		_port = other._port;
		_config = other._config;
	}
	return *this;
}

Listener::~Listener() {}

const ServerConfig* Listener::getConfig() const {
	return _config;
}

int Listener::getFd() const {
	return _fd;
}

const std::string& Listener::getHost() const {
	return _host;
}

int Listener::getPort() const {
	return _port;
}

void Listener::open() {
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw std::runtime_error("socket() failed for " + _host);

	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0
		|| setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed for " + _host);

	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl(O_NONBLOCK) failed for " + _host);

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<uint16_t>(_port));

	if (_host == "0.0.0.0")
		addr.sin_addr.s_addr = INADDR_ANY;
	else if (inet_pton(AF_INET, _host.c_str(), &addr.sin_addr) != 1)
		throw std::runtime_error("invalid host address: " + _host);

	if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed for " + _host + ":" + utils::toString(_port));

	if (listen(_fd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");
}

void Listener::close() {
	if (_fd >= 0) {
		::close(_fd);
		_fd = -1;
	}
}

void Listener::setConfig(const ServerConfig* cfg) {
	_config = cfg;
}
