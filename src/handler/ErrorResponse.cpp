#include "HTTPResponse.hpp"
#include "ConfigTypes.hpp"

#include <fstream>
#include <sstream>

template <typename T>
static bool loadErrorPage(const T& config, HTTPResponse& res, size_t code) {
	std::map<int, std::string>::const_iterator it = config.errorPages.find(code);
	if (it == config.errorPages.end() || it->second.empty())
		return false;

	std::string root = config.root;
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);

	std::ifstream f((root + "/" + it->second).c_str());
	if (!f.is_open())
		return false;

	std::ostringstream ss;
	ss << f.rdbuf();
	res.addHeader("Content-Type", "text/html");
	res.setBody(ss.str());

	return true;
}

template <typename T>
static HTTPResponse build(const T& config, size_t statusCode) {
	HTTPResponse res;
	res.setStatusCode(statusCode);
	res.addHeader("Connection", "close");

	if (loadErrorPage(config, res, statusCode))
		return res;

	std::ostringstream body;
	body << "<html><body><h1 style=\"text-center: align;\">"
		<< statusCode << " " << res.getReasonPhrase()
		<< "</h1></body></html>";
	res.addHeader("Content-Type", "text/html");
	res.setBody(body.str());

	return res;
}

HTTPResponse buildErrorResponse(const ServerConfig& config, size_t statusCode) {
	return build(config, statusCode);
}
HTTPResponse buildErrorResponse(const LocationConfig& config, size_t statusCode) {
	return build(config, statusCode);
}
