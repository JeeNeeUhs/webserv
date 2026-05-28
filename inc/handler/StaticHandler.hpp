#ifndef STATIC_HANDLER_HPP
# define STATIC_HANDLER_HPP

# include "ConfigTypes.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"

class StaticHandler {
	private:
		const ServerConfig* _config;

		bool pathExists(const std::string& path);
		bool isDirectory(const std::string& path);
		std::string mimeType(const std::string& path);
		HTTPResponse autoindex(const std::string& filePath, const std::string& requestPath);
		HTTPResponse serveFile(const std::string& filePath);
		HTTPResponse serveDirectory(const std::string& filePath, const std::string& requestPath,
			const LocationConfig& loc);

	public:
		StaticHandler();
		StaticHandler(const ServerConfig* other);
		StaticHandler(const StaticHandler& other);
		StaticHandler& operator=(const StaticHandler& other);
		~StaticHandler();

		HTTPResponse handleGet(const std::string& filePath, const std::string& requestPath,
			const LocationConfig& loc);
		// HTTPResponse handlePost(const HTTPRequest& req, const LocationConfig& loc);
		// HTTPResponse handleDelete(const std::string& filePath);
};

HTTPResponse buildErrorResponse(const ServerConfig* config, size_t statusCode);

#endif
