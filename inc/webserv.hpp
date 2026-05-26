#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>
# include <ctime>

# define PROGRAM_NAME "webserv"
# define DEFAULT_CONFIG_PATH "./config/default.conf"

# define HTTP_VERSION "1.1"

# define POLL_TIMEOUT 1000 // milliseconds (3 seconds)
# define READ_CHUNK 4096

namespace defaults {
	const size_t		CLIENT_MAX_BODY_SIZE	= 1024 * 1024;	// 1 MB
	const size_t		CLIENT_MAX_HEADER_SIZE	= 8 * 1024;		// 8 KB
	const time_t		CLIENT_HEADER_TIMEOUT	= 60;			// seconds
	const time_t		CLIENT_BODY_TIMEOUT		= 60;			// seconds
	const std::string	DEFAULT_ROOT			= ".";
	const std::string	DEFAULT_INDEX			= "index.html";
	const std::string	DEFAULT_HOST			= "0.0.0.0";
}

#endif
