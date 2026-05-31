#include "StaticHandler.hpp"

#include <sys/stat.h>
#include <dirent.h>
#include <sstream>

static bool pathExists(const std::string& path) {
	struct stat st;

	return stat(path.c_str(), &st) == 0;
}
 
static bool isDirectory(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	return S_ISDIR(st.st_mode);
}

static std::string mimeType(const std::string& path) {
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

static HTTPResponse autoindex(const LocationConfig& loc, const std::string& filePath, const std::string& requestPath) {
	DIR* dir = opendir(filePath.c_str());
	if (!dir)
		return buildErrorResponse(loc, 403);

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
		if (href.empty() || href[href.size() - 1] != '/')
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

static HTTPResponse serveFile(const LocationConfig& loc, const std::string& filePath) {
	struct stat st;
	if (stat(filePath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
		return buildErrorResponse(loc, 403);

	HTTPResponse res;

	res.setStatusCode(200);
	res.addHeader("Content-Type", mimeType(filePath));
	res.setFileBody(filePath, static_cast<size_t>(st.st_size));
	return res;
}

static HTTPResponse serveDirectory(const LocationConfig& loc, const std::string& filePath, const std::string& requestPath) {
	if (!loc.index.empty()) {
		std::string indexPath = filePath;

		if (indexPath.empty() || indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += loc.index;

		if (pathExists(indexPath))
			return serveFile(loc, indexPath);
	}

	if (loc.autoindex)
		return autoindex(loc, filePath, requestPath);

	return buildErrorResponse(loc, 403);
}

HTTPResponse StaticHandler::handleGet(const LocationConfig& loc, const std::string& filePath, const std::string& requestPath) {
	if (!pathExists(filePath))
		return buildErrorResponse(loc, 404);

	// if the path resolves to a directory but has no trailing slash, redirect to
	// the same path with a trailing slash so that relative urls in the served
	// page resolve correctly
	// for example index.js should be searched in /dir/index.js, not /index.js
	// 
	// https://datatracker.ietf.org/doc/html/rfc3986#section-5.2.2
	// https://github.com/nginx/nginx/blob/d44205284fa41662da803b796d6056fc1e59b1f3/src/http/modules/ngx_http_static_module.c#L148
	if (isDirectory(filePath) && (requestPath.empty() || requestPath[requestPath.size() - 1] != '/')) {
		HTTPResponse res;

		res.setStatusCode(301);
		res.addHeader("Location", requestPath + "/");
		return res;
	}

	if (isDirectory(filePath))
		return serveDirectory(loc, filePath, requestPath);

	return serveFile(loc, filePath);
}
