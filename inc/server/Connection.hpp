#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"

# include <string>

enum ConnState {
	READING_HEADERS,
	READING_BODY,

	STORING_BODY,
	CGI_REQUEST,
	HEADER_TIMEOUT,
	BODY_TIMEOUT,
	SEND_TIMEOUT,

	CONN_DONE
};

enum UploadState {
	UPLOAD_INIT,
	UPLOAD_FIND_BOUNDARY,
	UPLOAD_PARSE_HEADER,
	UPLOAD_WRITE_BODY
};

struct Connection {
	int listenFd;

	const ServerConfig*	config;
	ConnState			state;
	
	std::time_t	connStart;
	std::time_t	lastActivity;

	size_t			headerLength;
	std::string		readBuff;
	std::string		writeBuff;
	HTTPRequest		req;
	HTTPResponse	res;

	int		bodyFd;
	size_t	bodyRemaining;

	int			cgiPid;
	int			cgiReadFd;
	int			cgiWriteFd;
	std::string	cgiWriteBuff;

	UploadState					nmft; //firs time, no no not my first time
	std::string					boundary;
	std::string					uploadedFilename;
	const LocationConfig*		loc;
	std::vector<std::string>	uploadedFiles;
	bool						uploadEof;

	Connection()
		: listenFd(-1),
		config(NULL),
		state(READING_HEADERS),
		connStart(0),
		lastActivity(0),
		headerLength(0),
		bodyFd(-1),
		bodyRemaining(0),
		cgiPid(-1),
		cgiReadFd(-1),
		cgiWriteFd(-1),
		nmft(UPLOAD_INIT),
		uploadEof(false) {}
};

#endif
