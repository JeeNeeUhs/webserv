#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define PROGRAM_NAME "webserv"
# define DEFAULT_CONFIG_PATH "./config/default.conf"

# define HTTP_PROTOCOL "HTTP/1.0"

namespace defaults {
	const int			CLIENT_MAX_BODY_SIZE	= 1024 * 1024;	// 1 MB
	const int			CLIENT_MAX_HEADER_SIZE	= 8 * 1024;		// 8 KB
	const int			CLIENT_HEADER_TIMEOUT	= 60;			// seconds
	const int			CLIENT_BODY_TIMEOUT		= 60;			// seconds
	const char* const	DEFAULT_INDEX			= "index.html";
	const char* const	DEFAULT_HOST			= "0.0.0.0";
}

#endif
