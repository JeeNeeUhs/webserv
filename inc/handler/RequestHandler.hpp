#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"

class RequestHandler {
	private:
		const ServerConfig* _config;

		const LocationConfig* matchLocation(const std::string& path);
		bool isMethodAllowed(const std::string& method, const LocationConfig& loc);
		bool isCgiRequest(const std::string& path, const LocationConfig& loc);
		std::string resolvePath(const std::string& path, const LocationConfig& loc);

	public:
		RequestHandler();
		RequestHandler(const ServerConfig* config);
		RequestHandler(const RequestHandler& other);
		RequestHandler& operator=(const RequestHandler& other);
		~RequestHandler();

		HTTPResponse handle(const HTTPRequest& req);
};

HTTPResponse buildErrorResponse(const ServerConfig* config, size_t statusCode);

#endif
