#include "HTTPResponse.hpp"
#include "webserv.hpp"
#include "utils.hpp"

#include <sstream>

HTTPResponse::HTTPResponse() : _version(HTTP_VERSION), _isFileBody(false) {}

HTTPResponse::HTTPResponse(const HTTPResponse& other) {
	operator=(other);
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& other) {
	if (this != &other) {
		_version = other._version;
		_statusCode = other._statusCode;
		_reasonPhrase = other._reasonPhrase;
		_headers = other._headers;
		_body = other._body;
		_isFileBody = other._isFileBody;
		_filePath = other._filePath;
		_fileSize = other._fileSize;
	}

	return *this;
}

HTTPResponse::~HTTPResponse() {}

static std::string _getReasonPhrase(size_t statusCode) {
	switch (statusCode) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 408: return "Request Timeout";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 431: return "Request Header Fields Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown";
	}
}

std::string HTTPResponse::getReasonPhrase(void) const {
	return _reasonPhrase;
}

size_t HTTPResponse::getStatusCode(void) const {
	return _statusCode;
}

bool HTTPResponse::isFileBody(void) const {
	return _isFileBody;
}

const std::string& HTTPResponse::getFilePath(void) const {
	return _filePath;
}

size_t HTTPResponse::getFileSize(void) const {
	return _fileSize;
}

void HTTPResponse::setStatusCode(size_t statusCode) {
	_statusCode = statusCode;
	if (_reasonPhrase.empty())
		_reasonPhrase = _getReasonPhrase(statusCode);
}

void HTTPResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) {
	_body = body;
}

void HTTPResponse::setFileBody(const std::string& path, size_t size) {
	_isFileBody = true;
	_filePath = path;
	_fileSize = size;
}

std::string HTTPResponse::serialize(void) {
	std::ostringstream ss;

	ss << "HTTP/" << _version << ' ' << _statusCode << ' ' << _reasonPhrase << "\r\n";

	if (_headers.find("Content-Length") == _headers.end()) {
		size_t len = _isFileBody ? _fileSize : _body.size();
		addHeader("Content-Length", utils::toString(len));
	}
	// there is no keep-alive support
	if (_headers.find("Connection") == _headers.end())
		addHeader("Connection", "close");

	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
		ss << it->first << ": " << it->second << "\r\n";
	ss << "\r\n";

	std::string header = ss.str();
	if (_isFileBody)
		return header;

	return header + _body;
}
