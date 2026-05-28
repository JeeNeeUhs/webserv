#include "RequestHandler.hpp"
#include "StaticHandler.hpp"

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
static const LocationConfig* matchLocation(const ServerConfig* s, const std::string& path) {
	const LocationConfig* longest = NULL;
	size_t longestLen = 0;

	for (size_t i = 0; i < s->locations.size(); ++i) {
		const LocationConfig& loc = s->locations[i];
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

static bool isMethodAllowed(const std::string& method, const LocationConfig& loc) {
 	for (size_t i = 0; i < loc.methods.size(); ++i) {
		if (loc.methods[i] == method)
			return true;
	}

	return false;
}

static bool isCgiRequest(const std::string& path, const LocationConfig& loc) {
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
static std::string resolvePath(const std::string& path, const LocationConfig& loc) {
	std::string root = loc.root;
	if (!root.empty() && root[root.size() - 1] == '/')
		root = root.substr(0, root.size() - 1);
 
	return root + path;
}

HTTPResponse RequestHandler::handle(const ServerConfig* server, const HTTPRequest& req) {
	const LocationConfig* loc = matchLocation(server, req.getPath());
	if (!loc)
		return buildErrorResponse(server, 404);

	if (loc->redirect.first != 0) {
		HTTPResponse res;

		res.setStatusCode(loc->redirect.first);
		res.addHeader("Location", loc->redirect.second);
		return res;
	}

	if (!isMethodAllowed(req.getMethod(), *loc))
		return buildErrorResponse(loc, 405);

	std::string filePath = resolvePath(req.getPath(), *loc);

	if (isCgiRequest(req.getPath(), *loc)) {
		// TODO: cgi buraya artik
	}

	if (req.getMethod() == "GET")
		return StaticHandler::handleGet(loc, filePath, req.getPath());
	// else if (req.getMethod() == "POST")
	// 	return StaticHandler::handlePost();
 
	return buildErrorResponse(loc, 501); // not implemented
}
