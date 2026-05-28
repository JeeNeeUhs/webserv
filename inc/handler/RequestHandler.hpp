#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"

namespace RequestHandler {
	HTTPResponse handle(const ServerConfig* server, const HTTPRequest& req);
}

HTTPResponse buildErrorResponse(const ServerConfig* config, size_t statusCode);
HTTPResponse buildErrorResponse(const LocationConfig* config, size_t statusCode);

#endif
