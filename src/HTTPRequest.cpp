#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const HTTPRequest& other) {
	operator=(other);
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) {
	if (this != &other) {
		this->body = other.body;
	}

	return *this;
}

HTTPRequest::~HTTPRequest() {}

std::string HTTPRequest::getMethod() const {
	return method;
}

std::string HTTPRequest::getPath() const {
	return path;
}

std::string HTTPRequest::getProtocol() const {
	return protocol;
}

std::string HTTPRequest::getVersion() const {
	return version;
}

std::map<std::string, std::string> HTTPRequest::getQueries() const {
	return queries;
}

std::map<std::string, std::string> HTTPRequest::getHeaders() const {
	return headers;
}

std::string HTTPRequest::getBody() const {
	return body;
}

void HTTPRequest::fill1() {
	method = "GET";
	path = "/test";
	protocol = "HTTP";
	version = "1.1";
	queries["name"] = "value";
	headers["Content-Type"] = "text/html";
}

void HTTPRequest::fill2() {
	method = "GET";
	path = "/cgi-index";
	protocol = "HTTP";
	version = "1.1";
	queries["name"] = "value";
	headers["Content-Type"] = "text/html";
}

bool HTTPRequest::validate() {
	return true; //FIXME
}

bool HTTPRequest::parse(const std::string& rawRequest) {
	return true; //FIXME
}
