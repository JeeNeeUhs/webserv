#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "ConfigTypes.hpp"

# include <string>

enum ConnState {
	READING_HEADERS,
	READING_BODY,
	// WRITING_BODY,
	// PROCESSING,
	CONN_TIMEOUT,
	CONN_DONE
};

struct Connection {
	int listenFd;

	const ServerConfig*	config;
	ConnState			state;

	std::time_t	connStart;
	std::time_t	lastActivity;

	std::string	readBuff;
	std::string	writeBuff;

	size_t	headerLength;
	size_t	contentLength;

	Connection()
		: listenFd(-1),
		config(NULL),
		state(READING_HEADERS),
		connStart(0),
		lastActivity(0),
		headerLength(0),
		contentLength(0) {}
};

#endif
