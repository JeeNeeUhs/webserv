#include "ServerManager.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPParser.hpp"

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

		switch (c.state) {
			case READING_HEADERS:
				if (now - c.connStart > c.config->clientHeaderTimeout)
					c.state = HEADER_TIMEOUT;
				break;
			case READING_BODY:
				if (now - c.lastActivity > c.config->clientBodyTimeout)
					c.state = BODY_TIMEOUT;
				break;
			case CONN_DONE:
				if (now - c.lastActivity > c.config->clientBodyTimeout)
					c.state = SEND_TIMEOUT;
				break;
			default:
				break;
		}
	}
}

void ServerManager::acceptClients(int listenFd) {
	while (true) {
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EMFILE || errno == ENFILE) {
				Logger::warning("reached max fd limit, deferring accept()...");
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

	// TODO: MSG_NOSIGNAL
	int bytes = send(fd, c.writeBuff.c_str(), c.writeBuff.size(), 0);

	if (bytes > 0) {
		c.writeBuff.erase(0, bytes);

		if (c.writeBuff.empty()) {
			Logger::debug("fd " + utils::toString(fd) + " response sent");
			return false;
		}

		return true;
	}

	return true;
}

bool ServerManager::setErrorResponse(pollfd_t& pfd, Connection& c) {
	HTTPResponse res;

	res.setStatusCode(c.statusCode);
	res.addHeader("Connection", "close");
	res.addHeader("Content-Type", "text/plain");
	res.setBody(utils::toString(c.statusCode) + " error\n");

	c.writeBuff = res.serialize();
	c.state = CONN_DONE;
	c.lastActivity = std::time(NULL);
	pfd.events = POLLOUT;

	return true;
}

bool ServerManager::processBuffer(pollfd_t& pfd, Connection& c) {
	if (c.state == READING_HEADERS) {
		if (c.readBuff.size() > c.config->clientMaxHeaderSize) {
			c.statusCode = 413;
			return setErrorResponse(pfd, c);
		}

		std::size_t headerEnd = HTTPParser::findHeaderEnd(c.readBuff);
		if (headerEnd == std::string::npos)
			return true;

		c.headerLength = headerEnd;
		c.state = READING_BODY;
	}

	if (c.state == READING_BODY) {
		if (c.readBuff.size() - c.headerLength > c.config->clientMaxBodySize) {
			c.statusCode = 413;
			return setErrorResponse(pfd, c);
		}

		HTTPParser::RequestStatus status = HTTPParser::checkComplete(c.readBuff, c.headerLength);
		HTTPRequest req;

		if (status == HTTPParser::REQ_INCOMPLETE)
			return true;
		else if (status == HTTPParser::REQ_BAD || !req.parse(c.readBuff, c.headerLength)) {
			c.statusCode = 400;
			return setErrorResponse(pfd, c);
		}
		// c.req = req;

		// TODO: implement routing
		HTTPResponse res;
		res.setStatusCode(200);
		res.addHeader("Content-Type", "text/plain");
		res.addHeader("Connection", "close");
		res.setBody("webserv online panpa");
		c.writeBuff = res.serialize();

		c.state = CONN_DONE;
		pfd.events = POLLOUT;
	}

	return true;
}

bool ServerManager::readFromClient(pollfd_t& pfd, Connection& c) {
	char chunk[READ_CHUNK];

	while (true) {
		int bytes = recv(pfd.fd, chunk, sizeof(chunk), 0);

		if (bytes > 0) {
			size_t buffSize = c.readBuff.size() + bytes;
			size_t maxSize = c.config->clientMaxHeaderSize + c.config->clientMaxBodySize;
			if (buffSize > maxSize)
				break;

			c.readBuff.append(chunk, bytes);
		} else if (bytes == 0) {
			Logger::debug("fd " + utils::toString(pfd.fd) + " closed connection (eof)");
			return false;
		} else
			break;
	}

	return processBuffer(pfd, c);
}

bool ServerManager::handleClient(pollfd_t& pfd, short revents) {
	std::map<int, Connection>::iterator it = _connections.find(pfd.fd);
	if (it == _connections.end())
		return false;

	Connection& c = it->second;
	
	// RST packet, connection reset cases
	if (revents & (POLLERR | POLLNVAL)) {
		Logger::debug("fd " + utils::toString(pfd.fd) + " hung up");
		return false;
	}

	if (revents & (POLLIN | POLLHUP)) {
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
	std::ostringstream listens;

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

			listens << host << ":" << utils::toString(port) << " ";
		}
	}

	Logger::info("listening on " + listens.str());
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
				continue;
			}

			// clear timed out client connections
			std::map<int, Connection>::iterator it = _connections.find(pfd.fd);
			if (it != _connections.end()) {
				switch (it->second.state) {
					case HEADER_TIMEOUT:
					case BODY_TIMEOUT:
						it->second.statusCode = 408;
						setErrorResponse(pfd, it->second);
						continue;
					case SEND_TIMEOUT:
						close(pfd.fd);
						_connections.erase(it);
						pfd.fd = -1;
						continue;
					default:
						break;
				}
			}

			// there is no event to handle, skip for now
			if (pfd.revents == 0)
				continue;

			// process the incoming connections
			if (!handleClient(pfd, pfd.revents)) {
				close(pfd.fd);
				_connections.erase(pfd.fd);
				pfd.fd = -1;
			}
		}

		clearPollSet();
	}
}
