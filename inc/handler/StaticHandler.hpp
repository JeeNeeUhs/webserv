#ifndef STATIC_HANDLER_HPP
# define STATIC_HANDLER_HPP

# include "ConfigTypes.hpp"
# include "HTTPResponse.hpp"

namespace StaticHandler {
	HTTPResponse handleGet(const LocationConfig& loc, const std::string& filePath, const std::string& requestPath);
	HTTPResponse handleDelete(const LocationConfig& loc, const std::string& filePath);
}

HTTPResponse buildErrorResponse(const LocationConfig& config, size_t statusCode);

#endif
