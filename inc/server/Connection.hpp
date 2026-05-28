#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"
# include "RequestHandler.hpp"

# include <string>

enum ConnState {
	READING_HEADERS,
	READING_BODY,

	HEADER_TIMEOUT,
	BODY_TIMEOUT,
	SEND_TIMEOUT,

	CONN_DONE
};

struct Connection {
	int listenFd;

	const ServerConfig*	config;
	ConnState			state;
	RequestHandler		handler;

	std::time_t	connStart;
	std::time_t	lastActivity;

	size_t			headerLength;
	std::string		readBuff;
	std::string		writeBuff;
	HTTPRequest		req;
	HTTPResponse	res;

	Connection()
		: listenFd(-1),
		config(NULL),
		state(READING_HEADERS),
		connStart(0),
		lastActivity(0),
		headerLength(0) {}
};

#endif
