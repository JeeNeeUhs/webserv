#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stddef.h>

# define PROGRAM_NAME "webserv"
# define DEFAULT_CONFIG_PATH "./config/default.conf"

# define HTTP_PROTOCOL "HTTP/1.0"

# define POLL_TIMEOUT 3000 // milliseconds (3 seconds)
# define READ_CHUNK 4096

namespace defaults {
	const size_t		CLIENT_MAX_BODY_SIZE	= 1024 * 1024;	// 1 MB
	const size_t		CLIENT_MAX_HEADER_SIZE	= 8 * 1024;		// 8 KB
	const size_t		CLIENT_HEADER_TIMEOUT	= 60;			// seconds
	const size_t		CLIENT_BODY_TIMEOUT		= 60;			// seconds
	const char* const	DEFAULT_INDEX			= "index.html";
	const char* const	DEFAULT_HOST			= "0.0.0.0";
}

#endif
