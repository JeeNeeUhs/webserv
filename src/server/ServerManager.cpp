#include "ServerManager.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "webserv.hpp"
#include "HTTPParser.hpp"
#include "RequestHandler.hpp"
#include <signal.h>

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

ServerManager::ServerManager() {}

ServerManager::ServerManager(const std::vector<ServerConfig>& servers)
	: _servers(servers) {}

ServerManager::ServerManager(const ServerManager& other) {
	operator=(other);
}

ServerManager& ServerManager::operator=(const ServerManager& other) {
	if (this != &other) {
		_servers = other._servers;
		// fds are not copied, each ServerManager should own its own fds
		_pollFds.clear();
		_listeners.clear();
		_connections.clear();
	}
	return *this;
}

ServerManager::~ServerManager() {
	std::map<int, Connection>::iterator it;

	for (it = _connections.begin(); it != _connections.end(); ++it) {
		if (it->second.bodyFd != -1)
			close(it->second.bodyFd);
		close(it->first);
	}
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

void ServerManager::closeConnection(int& fd) {
	std::map<int, Connection>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		if (it->second.bodyFd != -1)
			close(it->second.bodyFd);

		if (it->second.cgiReadFd != -1) {
			close(it->second.cgiReadFd);
			for (size_t i = 0; i < _pollFds.size(); ++i) {
				if (_pollFds[i].fd == it->second.cgiReadFd) {
					_pollFds[i].fd = -1;
					break;
				}
			}
		}
		if (it->second.cgiWriteFd != -1) {
			close(it->second.cgiWriteFd);
			for (size_t i = 0; i < _pollFds.size(); ++i) {
				if (_pollFds[i].fd == it->second.cgiWriteFd) {
					_pollFds[i].fd = -1;
					break;
				}
			}
		}
		if (it->second.cgiPid != -1) {
			kill(it->second.cgiPid, SIGKILL);
			waitpid(it->second.cgiPid, NULL, 0);
		}
		
		_connections.erase(it);
	}

	close(fd);
	fd = -1;
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
	if (c.writeBuff.empty() && c.bodyFd != -1) {
		char chunkBuff[SEND_CHUNK];

		size_t chunkSize = sizeof(chunkBuff);
		if (chunkSize > c.bodyRemaining)
			chunkSize = c.bodyRemaining;

		int n = read(c.bodyFd, chunkBuff, chunkSize);
		if (n > 0) {
			c.writeBuff.append(chunkBuff, n);
			c.bodyRemaining -= n;
		}
		if (n <= 0 || c.bodyRemaining <= 0) {
			close(c.bodyFd);
			c.bodyFd = -1;
		}
	}

	if (c.writeBuff.empty() && c.cgiReadFd == -1)
		return false;
	
	int bytes = 0;
	if (!c.writeBuff.empty()) {
		bytes = send(fd, c.writeBuff.c_str(), c.writeBuff.size(), MSG_NOSIGNAL);
		Logger::debug("sent " + utils::toString(bytes) + " bytes to client fd " + utils::toString(fd));
		// Logger::debug(c.writeBuff);
	}
	if (bytes > 0) {
		c.writeBuff.erase(0, bytes);
		if (c.writeBuff.empty() && c.bodyFd == -1 && c.cgiReadFd == -1)
			return false;

		return true;
	}
	else if (bytes == 0 && c.cgiReadFd == -1){ // eof case, close connection
		Logger::debug("ben kapattim client fd " + utils::toString(fd));
		return false;
	}

	return true;
}

bool ServerManager::setErrorResponse(pollfd_t& pfd, Connection& c, size_t code) {
	c.res = buildErrorResponse(*c.config, code);
	c.writeBuff = c.res.serialize();

	// logging
	std::string method = c.req.getMethod().empty() ? "-" : c.req.getMethod();
	std::string path = c.req.getPath().empty() ? "-" : c.req.getPath();
	Logger::info(method + " " + path + " " + utils::toString(code));

	c.state = CONN_DONE;
	c.lastActivity = std::time(NULL);
	pfd.events = POLLOUT;

	return true;
}

bool ServerManager::processBuffer(pollfd_t& pfd, Connection& c) {
	if (c.state == READING_HEADERS) {
		if (c.readBuff.size() > c.config->clientMaxHeaderSize)
			return setErrorResponse(pfd, c, 431);

		std::size_t headerEnd = HTTPParser::findHeaderEnd(c.readBuff);
		if (headerEnd == std::string::npos)
			return true;
		c.headerLength = headerEnd;

		std::string path;
		std::string trash;
		HTTPParser::parseRequestLine(c.readBuff.substr(0, c.readBuff.find("\r\n")), trash, trash, path, trash, trash);
		if (RequestHandler::isCgiRequest(*RequestHandler::matchLocation(*c.config, path), path)) {
			c.state = CGI_REQUEST;
		} else {
		std::vector<std::string> ct = utils::split(HTTPParser::peekHeader(c.readBuff, "Content-Type"), ';');
		std::vector<std::string>::iterator it = std::find(ct.begin(), ct.end(), "multipart/form-data");
		if (it != ct.end())
			c.state = STORING_BODY;
		else
			c.state = READING_BODY;
		}
	}

	if (c.state == CGI_REQUEST) {
		if (c.cgiPid == -1 && c.cgiReadFd == -1 && c.cgiWriteFd == -1) {
			c.res = RequestHandler::createCgi(c);
			if (c.res.getStatusCode() != 0) {
				pfd.events = POLLOUT;	
				return true;
			}
			_cgiWriteFds[c.cgiWriteFd] = &c;
			_cgiReadFds[c.cgiReadFd] = &c;
			addPollFd(c.cgiReadFd, POLLIN);
			addPollFd(c.cgiWriteFd, POLLOUT);
		}
		return true;
	}


	if (c.state == STORING_BODY) {
		if (c.nmft == UPLOAD_INIT) {
			c.res = RequestHandler::validateUploadRequest(c);
			if (c.nmft == UPLOAD_INIT) {
				c.writeBuff = c.res.serialize();
				c.state = CONN_DONE;
				pfd.events = POLLOUT;
				return true;
			}	
		}
		c.res = RequestHandler::uploadToStore(c);
		if (c.res.getStatusCode() != 0) {
			c.writeBuff = c.res.serialize();
			c.state = CONN_DONE;
			pfd.events = POLLOUT;
			return true;
		}
	}

	if (c.state == READING_BODY) {
		if (c.readBuff.size() - c.headerLength > c.config->clientMaxBodySize)
			return setErrorResponse(pfd, c, 413);

		HTTPParser::RequestStatus status = HTTPParser::checkComplete(c.readBuff, c.headerLength);
		if (status == HTTPParser::REQ_INCOMPLETE)
			return true;
		else if (status == HTTPParser::REQ_BAD || !c.req.parse(c.readBuff, c.headerLength))
			return setErrorResponse(pfd, c, 400);

		c.res = RequestHandler::handle(*c.config, c.req);
		if (c.res.isFileBody()) {
			c.bodyFd = open(c.res.getFilePath().c_str(), O_RDONLY);

			if (c.bodyFd < 0)
				c.res = buildErrorResponse(*c.config, 500);
			else
				c.bodyRemaining = c.res.getFileSize();
		}

		Logger::info(c.req.getMethod() + " "
			+ c.req.getPath()
			+ (c.req.getQuery().empty() ? "" : "?" + c.req.getQuery())
			+ " " + utils::toString(c.res.getStatusCode()));

		c.writeBuff = c.res.serialize();
		c.state = CONN_DONE;
		pfd.events = POLLOUT;
	}

	return true;
}

bool ServerManager::readFromClient(pollfd_t& pfd, Connection& c) {
	char chunk[RECV_CHUNK];

	int bytes = recv(pfd.fd, chunk, sizeof(chunk), 0);
	if (bytes > 0) {
		size_t buffSize = c.readBuff.size() + bytes;
		size_t maxSize = c.config->clientMaxHeaderSize + c.config->clientMaxBodySize;
		if (buffSize > maxSize)
			return processBuffer(pfd, c);

		c.readBuff.append(chunk, bytes);
	} else if (bytes == 0) {
		Logger::debug("fd " + utils::toString(pfd.fd) + " closed connection (eof)");
		if (c.state == STORING_BODY) {
			c.uploadEof = true;
			c.res = RequestHandler::uploadToStore(c);
			c.writeBuff = c.res.serialize();
			c.state = CONN_DONE;
			pfd.events = POLLOUT;
			return true;
		}
		return false;
	}

	return processBuffer(pfd, c);
}

bool ServerManager::readFromCgi(pollfd_t& pfd, Connection& c) {
	char chunk[RECV_CHUNK];

	int bytes = read(pfd.fd, chunk, sizeof(chunk));
	if (bytes > 0 && c.nmft == UPLOAD_WRITE_BODY) {
		std::string temp(chunk, bytes);
		c.writeBuff.append(utils::toChunked(temp));
		Logger::debug("read " + utils::toString(bytes) + " bytes from cgi for fd " + utils::toString(pfd.fd));
		return true;
	} else if (bytes > 0 && c.nmft != UPLOAD_WRITE_BODY) {
		c.cgiWriteBuff.append(chunk, bytes);
		Logger::debug("read " + utils::toString(bytes) + " bytes from cgi for fd " + utils::toString(pfd.fd));
		std::size_t headerEnd = HTTPParser::findHeaderEnd(c.cgiWriteBuff);
		if (headerEnd != std::string::npos) {
			pollfd_t *tempPfd = getConnetionPfd(c);
			RequestHandler::cgiDone(c, *tempPfd);
			Logger::debug("cgi process for fd " + utils::toString(pfd.fd) + " header finished");
		}
	} else if (bytes == 0) {
		c.writeBuff.append(utils::toChunked(""));
		Logger::debug("cgi process for fd " + utils::toString(pfd.fd) + " finished reading");
		return false;
	}

	return true;
}

bool ServerManager::writeToCgi(pollfd_t& pfd, Connection& c) {
	if (c.readBuff.empty())
		return true;

	int bytes = write(pfd.fd, c.readBuff.c_str(), c.readBuff.size());
	if (bytes > 0) {
		Logger::debug("wrote " + utils::toString(bytes) + " bytes to cgi for fd " + utils::toString(pfd.fd));
		c.readBuff.erase(0, bytes);
		return true;
	} else if (bytes == 0) {
		Logger::debug("cgi process for fd " + utils::toString(pfd.fd) + " finished writing");
		return false;
	}

	return true;
}

bool ServerManager::handleClient(pollfd_t& pfd, short revents) {
	// check if the event is for a cgi read/write fd
	std::map<int, Connection*>::iterator cgiIt;
	cgiIt = _cgiReadFds.find(pfd.fd);
	if (cgiIt != _cgiReadFds.end()) {
		
		if (!readFromCgi(pfd, *cgiIt->second)){
			Logger::debug("calistim1");

			if (cgiIt->second->state != CONN_DONE) {
				cgiIt->second->res = buildErrorResponse(*cgiIt->second->config, 500);
				cgiIt->second->writeBuff = cgiIt->second->res.serialize();
				cgiIt->second->state = CONN_DONE;
				pollfd_t *tempPfd = ServerManager::getConnetionPfd(*cgiIt->second);
				tempPfd->events = POLLOUT;
				Logger::debug("cgi process for fd " + utils::toString(pfd.fd) + " did not return with header, sending 500 response");
			}
			RequestHandler::cgiDoneReading(*cgiIt->second, pfd);
		}
		return true;
	}
	cgiIt = _cgiWriteFds.find(pfd.fd);
	if (cgiIt != _cgiWriteFds.end()) {
		if (!writeToCgi(pfd, *cgiIt->second))
			RequestHandler::cgiDoneWriting(*cgiIt->second, pfd);
		return true;
	}
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

pollfd_t* ServerManager::getConnetionPfd(Connection& c) {
	std::map<int, Connection>::iterator it;
	for (it = _connections.begin(); it != _connections.end(); ++it) {
		if (&it->second == &c) {
			for (size_t i = 0; i < _pollFds.size(); ++i) {
				if (_pollFds[i].fd == it->first)
					return &_pollFds[i];
			}
			break;
		}
	}
	return NULL;
}

void ServerManager::setup(void) {
	std::ostringstream listens;

	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& config = _servers[i];

		for (size_t j = 0; j < config.listens.size(); ++j) {
			const std::string& host = config.listens[j].first;
			int port = config.listens[j].second;

			Listener l(&config, host, port);
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
						setErrorResponse(pfd, it->second, 408);
						continue;
					case SEND_TIMEOUT:
						closeConnection(pfd.fd);
						continue;
					default:
						break;
				}
			}

			// there is no event to handle, skip for now
			if (pfd.revents == 0)
				continue;

			// process the incoming connections
			if (!handleClient(pfd, pfd.revents))
				closeConnection(pfd.fd);
		}

		clearPollSet();
	}
}
