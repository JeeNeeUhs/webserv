#include "HTTPRequestBody.hpp"

HTTPRequestBody::HTTPRequestBody() {}

HTTPRequestBody::HTTPRequestBody(const HTTPRequestBody& other) {
	operator=(other);
}

HTTPRequestBody& HTTPRequestBody::operator=(const HTTPRequestBody& other) {
	if (this != &other) {
		this->body = other.body;
	}

	return *this;
}

HTTPRequestBody::~HTTPRequestBody() {}

std::string HTTPRequestBody::getMethod() const {
	return method;
}

std::string HTTPRequestBody::getPath() const {
	return path;
}

std::string HTTPRequestBody::getProtocol() const {
	return protocol;
}

std::string HTTPRequestBody::getVersion() const {
	return version;
}

std::map<std::string, std::string> HTTPRequestBody::getHeaders() const {
	return headers;
}

std::string HTTPRequestBody::getBody() const {
	return body;
}

bool HTTPRequestBody::validate() {
	return true; //FIXME
}

bool HTTPRequestBody::parse(const std::string& rawRequest) {
	return true; //FIXME
}
