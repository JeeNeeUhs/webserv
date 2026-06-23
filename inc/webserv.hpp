#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <string>
# include <ctime>

# define PROGRAM_NAME "webserv"
# define DEFAULT_CONFIG_PATH "./config/default.conf"

# define HTTP_VERSION "1.1"

# define POLL_TIMEOUT 1000 // milliseconds (3 seconds)
# define CGI_TIMEOUT 30
# define SESSION_TIMEOUT 60
# define RECV_CHUNK 4 * 1024
# define SEND_CHUNK 64 * 1024

namespace defaults {
	const size_t		CLIENT_MAX_BODY_SIZE	= 1024 * 1024;	// 1 MB
	const time_t		CLIENT_BODY_TIMEOUT		= 60;			// seconds
	const std::string	DEFAULT_ROOT			= "/tmp/webserv";
	const std::string	DEFAULT_INDEX			= "index.html";
	const std::string	DEFAULT_HOST			= "0.0.0.0";
}

namespace CGI {
	const std::string	SERVER_SOFTWARE			= "webserv/1.0";
	const std::string	SERVER_PROTOCOL			= "HTTP/1.1";
	const std::string	GATEWAY_INTERFACE		= "CGI/1.1";
}

#endif
