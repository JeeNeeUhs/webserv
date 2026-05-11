#include "HTTPResponse.hpp"
#include "webserv.hpp"

HTTPResponse::HTTPResponse() : _version(HTTP_PROTOCOL) {}

HTTPResponse::HTTPResponse(int statusCode, const std::string& reasonPhrase)
	: _version(HTTP_PROTOCOL), _statusCode(statusCode), _reasonPhrase(reasonPhrase) {}

HTTPResponse::HTTPResponse(int statusCode, const std::string& reasonPhrase, const std::map<std::string, std::string>& headers, const std::string& body)
	: _version(HTTP_PROTOCOL), _statusCode(statusCode), _reasonPhrase(reasonPhrase), _headers(headers), _body(body) {}

HTTPResponse::HTTPResponse(const HTTPResponse& other)
	: _version(HTTP_PROTOCOL), _statusCode(other._statusCode), _reasonPhrase(other._reasonPhrase), _headers(other._headers), _body(other._body) {}

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

void HTTPResponse::setStatusCode(int statusCode) {
	_statusCode = statusCode;
}

void HTTPResponse::setReasonPhrase(const std::string& reasonPhrase) {
	_reasonPhrase = reasonPhrase;
}

void HTTPResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) {
	_body = body;
}

const std::string HTTPResponse::toString() const {
	std::string response = _version + " " + std::to_string(_statusCode) + " " + _reasonPhrase + "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n" + _body;
	return response;
}
