#include "ServerManager.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "webserv.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

ServerManager::ServerManager() {}

ServerManager::ServerManager(const std::vector<ServerConfig>& configs)
	: _configs(configs) {}

ServerManager::ServerManager(const ServerManager& other) {
	operator=(other);
}

ServerManager& ServerManager::operator=(const ServerManager& other) {
	if (this != &other) {
		_configs = other._configs;
		// fds are not copied, each ServerManager should own its own fds
		_pollFds.clear();
		_listeners.clear();
		_connections.clear();
	}
	return *this;
}

ServerManager::~ServerManager() {
	std::map<int, Connection>::iterator it;

	for (it = _connections.begin(); it != _connections.end(); ++it)
		close(it->first);
	_connections.clear();

	for (size_t i = 0; i < _listeners.size(); ++i)
		_listeners[i].close();
}

void ServerManager::addPollFd(int fd, short events) {
	pollfd_t pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;

	_pollFds.push_back(pfd);
}

void ServerManager::clearPollSet(void) {
	for (size_t i = 0; i < _pollFds.size(); ) {
		if (_pollFds[i].fd == -1) {
			// using pop + swap operations instead of erase
			// with O(1) complexity
			_pollFds[i] = _pollFds.back();
			_pollFds.pop_back();
		} else
			++i;
	}
}

void ServerManager::checkTimeouts(void) {
	std::time_t now = std::time(NULL);

	std::map<int, Connection>::iterator it;
	for (it = _connections.begin(); it != _connections.end(); ++it) {
		Connection& c = it->second;

		if (c.state == CONN_TIMEOUT)
			continue;

		bool headerTimeout = (c.state == READING_HEADERS
			&& (now - c.connStart > static_cast<time_t>(c.config->clientHeaderTimeout)));
		bool bodyTimeout = (c.state == READING_BODY
			&& (now - c.lastActivity > static_cast<time_t>(c.config->clientBodyTimeout)));
		bool sendTimeout = (c.state == CONN_DONE
			&& (now - c.lastActivity > static_cast<time_t>(c.config->clientBodyTimeout)));

		if (headerTimeout || bodyTimeout || sendTimeout)
 			c.state = CONN_TIMEOUT;
	}
}

void ServerManager::acceptClients(int listenFd) {
	while (true) {
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EMFILE || errno == ENFILE) {
				Logger::warning("reached max fd limit, deferring accept...");
				break;
			}

			continue;
		}

		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
			close(clientFd);
			continue;
		}

		Connection c;
		c.listenFd = listenFd;
		c.config = _listeners[listenFd].getConfig();
		c.connStart = std::time(NULL);
		c.lastActivity = c.connStart;

		_connections[clientFd] = c;
		addPollFd(clientFd, POLLIN);

		Logger::debug("new client fd " + utils::toString(clientFd)
			+ " on listen fd " + utils::toString(listenFd));
	}
}


bool ServerManager::sendToClient(int fd, Connection& c) {
	if (c.writeBuff.empty())
		return false;

	int bytes = send(fd, c.writeBuff.c_str(), c.writeBuff.size(), 0);

	if (bytes > 0) {
		c.writeBuff.erase(0, bytes);

		if (c.writeBuff.empty()) {
			Logger::debug("fd " + utils::toString(fd) + " response sent");

			return false;
		}

		return true;
	}

	Logger::debug("fd " + utils::toString(fd) + " send error");
	return false;
}

bool ServerManager::readFromClient(pollfd_t& pfd, Connection& c) {
	char chunk[READ_CHUNK];

	while (true) {
		int bytes = recv(pfd.fd, chunk, sizeof(chunk), 0);

		if (bytes > 0) {
			const ServerConfig* cfg = c.config;
			size_t total = c.readBuff.size() + static_cast<size_t>(bytes);

			if (c.state == READING_HEADERS && total > cfg->clientMaxHeaderSize) {
				return false; // TODO: 431 response
			} else if (c.state == READING_BODY) {
				size_t bodySize = total - c.headerLength;

				if (bodySize > c.contentLength || bodySize > cfg->clientMaxBodySize)
					return false; // TODO: 413 response
			}

			c.readBuff.append(chunk, bytes);
		} else if (bytes == 0) {
			Logger::debug("fd " + utils::toString(pfd.fd) + "closed connection (EOF)");
			return false;
		} else
			break;
	}

	if (c.state == READING_HEADERS) {
		size_t pos = c.readBuff.find("\r\n\r\n");

		if (pos != std::string::npos) {
			c.headerLength = pos + 4;
			c.state = READING_BODY;

			// TODO: header parsing
		}
	}

	if (c.state == READING_BODY) {
		size_t bodySize = c.readBuff.size() - c.headerLength;

		if (bodySize >= c.contentLength) {
			c.state = CONN_DONE;

			c.writeBuff =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 12\r\n"
				"Connection: close\r\n"
				"\r\n"
				"Hello World!";

			pfd.events = POLLOUT;
		}

		// TODO: prep http response
	}

	return true;
}

bool ServerManager::handleClient(pollfd_t& pfd, short revents) {
	std::map<int, Connection>::iterator it = _connections.find(pfd.fd);
	if (it == _connections.end())
		return false;

	Connection& c = it->second;
	
	// RST packet, hang up cases
	if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
		Logger::debug("fd " + utils::toString(pfd.fd) + " hung up");
		return false;
	}

	if (revents & POLLIN) {
		if (!readFromClient(pfd, c))
			return false;
		c.lastActivity = std::time(NULL);
	}

	if (revents & POLLOUT) {
		if (!sendToClient(pfd.fd, c))
			return false;
		c.lastActivity = std::time(NULL);
	}

	return true;
}

void ServerManager::setup(void) {
	for (size_t i = 0; i < _configs.size(); ++i) {
		const ServerConfig& cfg = _configs[i];

		for (size_t j = 0; j < cfg.listens.size(); ++j) {
			const std::string& host = cfg.listens[j].first;
			int port = cfg.listens[j].second;

			Listener l(host, port);
			l.setConfig(&cfg);
			l.open();

			int lFd = l.getFd();
			_listeners[lFd] = l;
			addPollFd(lFd, POLLIN);

			Logger::info("listening on "
				+ host + ":" + utils::toString(port) + ", fd: " + utils::toString(lFd));
		}
	}
}

void ServerManager::run(void) {
	while (true) {
		checkTimeouts();

		int ready = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT);

		if (ready < 0) {
			if (errno == EINTR)
				continue;
			Logger::error(std::string("poll() failed: ") + std::strerror(errno));
			break;
		}

		for (size_t i = 0; i < _pollFds.size(); ++i) {
			pollfd_t& pfd = _pollFds[i];
			if (pfd.fd == -1)
				continue;

			// accept the new connection
			if (_listeners.find(pfd.fd) != _listeners.end()) {
				if (pfd.revents & POLLIN)
					acceptClients(pfd.fd);
				pfd.revents = 0;
				continue;
			}

			// clear timed out client connections
			std::map<int, Connection>::iterator it = _connections.find(pfd.fd);
			if (it != _connections.end() && it->second.state == CONN_TIMEOUT) {
				Logger::debug("fd " + utils::toString(pfd.fd) + " timed out");

				close(pfd.fd);
				_connections.erase(it);
				pfd.fd = -1;

				continue;
			}

			if (pfd.revents == 0)
				continue;

			// process the incoming connections
			if (!handleClient(pfd, pfd.revents)) {
				close(pfd.fd);
				_connections.erase(pfd.fd);
				pfd.fd = -1;
			}

			pfd.revents = 0;
		}

		clearPollSet();
	}
}
