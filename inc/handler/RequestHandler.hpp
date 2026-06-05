#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"
# include "Connection.hpp"

namespace RequestHandler {
	const LocationConfig* matchLocation(const ServerConfig& server, const std::string& path);
	bool isCgiRequest(const LocationConfig& loc, const std::string& path);
	HTTPResponse validateUploadRequest(Connection& c);
	HTTPResponse uploadToStore(Connection& c);
	HTTPResponse doneCgi(Connection& c);
	void doneWritingCgi(Connection& c);
	HTTPResponse createCgi(Connection& c);
	HTTPResponse handle(const ServerConfig& server, const HTTPRequest& req);
}

HTTPResponse buildErrorResponse(const ServerConfig& config, size_t statusCode);
HTTPResponse buildErrorResponse(const LocationConfig& config, size_t statusCode);

#endif
