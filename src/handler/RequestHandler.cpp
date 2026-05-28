#include "RequestHandler.hpp"
#include "StaticHandler.hpp"

RequestHandler::RequestHandler() : _config(NULL) {}

RequestHandler::RequestHandler(const ServerConfig* config)
	: _config(config) {}

RequestHandler::RequestHandler(const RequestHandler& other) {
	operator=(other);
}

RequestHandler& RequestHandler::operator=(const RequestHandler& other) {
	if (this != &other)
		_config = other._config;

	return *this;
}

RequestHandler::~RequestHandler() {}

// iterates all locations and returns the longest prefix of the request path.
// config: location /, location /api, location /api/users
// 
// GET /				-> matches with /
// GET /about			-> matches with /
// GET /ap				-> matches with /
// GET /apitest			-> matches with /
// GET /api				-> matches with /, /api	-> chooses /api (longest one)
// GET /api/v2			-> matches with /, /api	-> chooses /api
// GET /api/users/42	-> mathces all of them	-> chooses /api/users
const LocationConfig* RequestHandler::matchLocation(const std::string& path) {
	const LocationConfig* longest = NULL;
	size_t longestLen = 0;

	for (size_t i = 0; i < _config->locations.size(); ++i) {
		const LocationConfig& loc = _config->locations[i];
		const std::string& locPath = loc.path;

		if (path.find(locPath) != 0)
			continue;
		
		// boundary check, /apitest doesnt match with /api
		// or /api doesnt match with /api/users
		if (locPath != "/"
			&& path.size() > locPath.size() && path[locPath.size()] != '/')
			continue;

		if (locPath.size() > longestLen) {
			longestLen = locPath.size();
			longest = &loc;
		}
	}

	return longest;
}

bool RequestHandler::isMethodAllowed(const std::string& method, const LocationConfig& loc) {
 	for (size_t i = 0; i < loc.methods.size(); ++i) {
		if (loc.methods[i] == method)
			return true;
	}

	return false;
}

bool RequestHandler::isCgiRequest(const std::string& path, const LocationConfig& loc) {
	if (loc.cgiExtensions.empty())
		return false;
 
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return false;
 
	std::string ext = path.substr(dot);
	for (size_t i = 0; i < loc.cgiExtensions.size(); ++i) {
		if (loc.cgiExtensions[i] == ext)
			return true;
	}

	return false;
}

// if URL /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet
// will search for /tmp/www/pouic/toto/pouet
std::string RequestHandler::resolvePath(const std::string& path, const LocationConfig& loc) {
	std::string root = loc.root;
	if (!root.empty() && root[root.size() - 1] == '/')
		root = root.substr(0, root.size() - 1);
 
	return root + path;
}

HTTPResponse RequestHandler::handle(const HTTPRequest& req) {
	const LocationConfig* loc = matchLocation(req.getPath());
	if (!loc)
		return buildErrorResponse(_config, 404);

	if (loc->redirect.first != 0) {
		HTTPResponse res;

		res.setStatusCode(loc->redirect.first);
		res.addHeader("Location", loc->redirect.second);
		return res;
	}

	if (!isMethodAllowed(req.getMethod(), *loc))
		return buildErrorResponse(_config, 405);

	if (isCgiRequest(req.getPath(), *loc)) {
		HTTPResponse res;
		res.setStatusCode(0);
		return res;
	}

	StaticHandler sHandler(_config);
	std::string filePath = resolvePath(req.getPath(), *loc);

	if (req.getMethod() == "GET")
		return sHandler.handleGet(filePath, req.getPath(), *loc);
	// else if (req.getMethod() == "POST")
	// 	return sHandler.handlePost(req, *loc);
	// else if (req.getMethod() == "DELETE")
	// 	return sHandler.handleDelete(filePath);
 
	return buildErrorResponse(_config, 501); // not implemented
}
