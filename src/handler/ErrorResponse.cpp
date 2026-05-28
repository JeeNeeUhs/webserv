#include "HTTPResponse.hpp"
#include "ConfigTypes.hpp"

#include <fstream>
#include <sstream>

HTTPResponse buildErrorResponse(const ServerConfig* config, size_t statusCode) {
	HTTPResponse res;
	res.setStatusCode(statusCode);
	res.addHeader("Connection", "close");

	std::map<int, std::string>::const_iterator it = config->errorPages.find(statusCode);
	if (it != config->errorPages.end() && !it->second.empty()) {
		std::string root = config->root;
		// remove the / at the end if exists
		if (!root.empty() && root[root.size() - 1] == '/')
			root = root.substr(0, root.size() - 1);

		std::string pagePath = root + "/" + it->second;
		std::ifstream f(pagePath.c_str());
		if (f.is_open()) {
			std::ostringstream ss;

			ss << f.rdbuf();
			res.addHeader("Content-Type", "text/html");
			res.setBody(ss.str());
			return res;
		}
	}

	std::ostringstream body;

	// TODO: assets/ altinda bunlari defaultErrorPages gibisinden tutariz
	body << "<html><body><h1>" << statusCode << "</h1></body></html>";
	res.addHeader("Content-Type", "text/html");
	res.setBody(body.str());
	return res;
}
