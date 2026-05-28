#include "StaticHandler.hpp"

#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

StaticHandler::StaticHandler() : _config(NULL) {}

StaticHandler::StaticHandler(const ServerConfig* config) : _config(config) {}

StaticHandler::StaticHandler(const StaticHandler& other) {
	operator=(other);
}

StaticHandler& StaticHandler::operator=(const StaticHandler& other) {
	if (this != &other)
		_config = other._config;

	return *this;
}

StaticHandler::~StaticHandler() {}

bool StaticHandler::pathExists(const std::string& path) {
	struct stat st;

	return stat(path.c_str(), &st) == 0;
}
 
bool StaticHandler::isDirectory(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	return S_ISDIR(st.st_mode);
}

std::string StaticHandler::mimeType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
 
	std::string ext = path.substr(dot);
	if (ext == ".html" || ext == ".htm")
		return "text/html";
	if (ext == ".css")
		return "text/css";
	if (ext == ".js")
		return "application/javascript";
	if (ext == ".json")
		return "application/json";
	if (ext == ".png")
		return "image/png";
	if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	if (ext == ".gif")
		return "image/gif";
	if (ext == ".svg")
		return "image/svg+xml";
	if (ext == ".ico")
		return "image/x-icon";
	if (ext == ".txt")
		return "text/plain";
	if (ext == ".pdf")
		return "application/pdf";
	if (ext == ".xml")
		return "text/xml";
	if (ext == ".webp")
		return "image/webp";
	if (ext == ".mp4")
		return "video/mp4";
	if (ext == ".webm")
		return "video/webm";
	return "application/octet-stream";
}

HTTPResponse StaticHandler::autoindex(const std::string& filePath, const std::string& requestPath) {
	DIR* dir = opendir(filePath.c_str());
	if (!dir) {
		HTTPResponse res;

		res.setStatusCode(403);
		return res;
	}

	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><title>Index of "
		<< requestPath << "</title></head><body>\n"
		<< "<h1>Index of " << requestPath << "</h1><hr><pre>\n";
 
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == ".")
			continue;
 
		std::string href = requestPath;
		if (href[href.size() - 1] != '/')
			href += '/';
		href += name;
 
		html << "<a href=\"" << href << "\">" << name << "</a>\n";
	}
	closedir(dir);
 
	html << "</pre><hr></body></html>\n";
 
	HTTPResponse res;

	res.setStatusCode(200);
	res.addHeader("Content-Type", "text/html");
	res.setBody(html.str());
	return res;
}

HTTPResponse StaticHandler::serveFile(const std::string& filePath) {
	// open as binary, not regular text mode
	std::ifstream f(filePath.c_str(), std::ios::binary);
	if (!f.is_open())
		return buildErrorResponse(_config, 403);
 
	std::ostringstream ss;
	ss << f.rdbuf();
 
	HTTPResponse res;

	res.setStatusCode(200);
	res.addHeader("Content-Type", mimeType(filePath));
	res.setBody(ss.str());
	return res;
}

HTTPResponse StaticHandler::serveDirectory(const std::string& filePath, const std::string& requestPath,
	const LocationConfig& loc) { 
	if (!loc.index.empty()) {
		std::string indexPath = filePath;

		if (indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += loc.index;
 
		if (pathExists(indexPath))
			return serveFile(indexPath);
	}

	if (loc.autoindex)
		return autoindex(filePath, requestPath);
 
	return buildErrorResponse(_config, 403);
}

HTTPResponse StaticHandler::handleGet(const std::string& filePath, const std::string& requestPath,
	const LocationConfig& loc) {
	if (!pathExists(filePath))
		return buildErrorResponse(_config, 404);

	// if the path resolves to a directory but has no trailing slash, redirect to
	// the same path with a trailing slash so that relative urls in the served
	// page resolve correctly
	// for example index.js should be searched in /dir/index.js, not /index.js
	// 
	// https://datatracker.ietf.org/doc/html/rfc3986#section-5.2.2
	// https://github.com/nginx/nginx/blob/d44205284fa41662da803b796d6056fc1e59b1f3/src/http/modules/ngx_http_static_module.c#L148
	else if (isDirectory(filePath) && requestPath[requestPath.size() - 1] != '/') {
		HTTPResponse res;

		res.setStatusCode(301);
		res.addHeader("Location", requestPath + "/");
		return res;
	}

	else if (isDirectory(filePath))
		return serveDirectory(filePath, requestPath, loc);

	return serveFile(filePath);
}	

/*
HTTPResponse StaticHandler::handlePost(const HTTPRequest& req, const LocationConfig& loc) {
}

HTTPResponse StaticHandler::handleDelete(const std::string& filePath) {
}
*/