#include "HTTPResponse.hpp"
#include "webserv.hpp"
#include "utils.hpp"

#include <sstream>

HTTPResponse::HTTPResponse() : _version(HTTP_VERSION) {}

HTTPResponse::HTTPResponse(const HTTPResponse& other) {
	operator=(other);
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& other) {
	if (this != &other) {
		_statusCode = other._statusCode;
		_reasonPhrase = other._reasonPhrase;
		_headers = other._headers;
		_body = other._body;
	}

	return *this;
}

HTTPResponse::~HTTPResponse() {}

static std::string reasonFor(int code) {
	switch (code) {
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

void HTTPResponse::setStatusCode(int statusCode) {
	_statusCode = statusCode;
	if (_reasonPhrase.empty())
		_reasonPhrase = reasonFor(statusCode);
}

void HTTPResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) {
	_body = body;
}

std::string HTTPResponse::serialize(void) {
	std::ostringstream ss;
	ss << "HTTP/" << _version << ' ' << _statusCode << ' ' << _reasonPhrase << "\r\n";

	if (_headers.find("Content-Length") == _headers.end())
		addHeader("Content-Length", utils::toString(_body.size()));

	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
		ss << it->first << ": " << it->second << "\r\n";

	ss << "\r\n" << _body;
	return ss.str();
}
