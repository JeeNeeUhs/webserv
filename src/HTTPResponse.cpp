#include "HTTPRequest.hpp"
#include <cstdlib>
#include <cctype>

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const HTTPRequest& other) {
	operator=(other);
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) {
	if (this != &other) {
		_method = other._method;
		_path = other._path;
		_protocol = other._protocol;
		_version = other._version;
		_queries = other._queries;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

HTTPRequest::~HTTPRequest() {}

const std::string& HTTPRequest::getMethod() const {
	return _method;
}

const std::string& HTTPRequest::getPath() const {
	return _path;
}

const std::string& HTTPRequest::getProtocol() const {
	return _protocol;
}

const std::string& HTTPRequest::getVersion() const {
	return _version;
}

const std::map<std::string, std::string>& HTTPRequest::getQueries() const {
	return _queries;
}

const std::map<std::string, std::string>& HTTPRequest::getHeaders() const {
	return _headers;
}

const std::string& HTTPRequest::getBody() const {
	return _body;
}

std::string HTTPRequest::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;

	std::string lowerKey = safeToLower(key);
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		if (safeToLower(it->first) == lowerKey)
			return it->second;
	}
	return "";
}

void HTTPRequest::_parseQueryString(const std::string& queryStr) {
	size_t pos = 0;
	while (pos <= queryStr.size()) {
		size_t ampPos = queryStr.find('&', pos);
		if (ampPos == std::string::npos)
			ampPos = queryStr.size();

		std::string pair = queryStr.substr(pos, ampPos - pos);
		if (!pair.empty()) {
			size_t eqPos = pair.find('=');
			if (eqPos != std::string::npos) {
				_queries[urlDecode(pair.substr(0, eqPos))] = urlDecode(pair.substr(eqPos + 1));
			} else {
				_queries[urlDecode(pair)] = "";
			}
		}
		if (ampPos == queryStr.size())
			break;
		pos = ampPos + 1;
	}
}

bool HTTPRequest::parse(const std::string& rawRequest) {
	_method.clear();
	_path.clear();
	_protocol.clear();
	_version.clear();
	_queries.clear();
	_headers.clear();
	_body.clear();

	size_t headerEnd = rawRequest.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	size_t lineEnd = rawRequest.find("\r\n");
	if (lineEnd == std::string::npos || lineEnd > headerEnd)
		return false;

	if (!parseRequestLine(rawRequest.substr(0, lineEnd), _method, _path, _protocol, _version))
		return false;

	std::string rawHeaders = rawRequest.substr(lineEnd + 2, headerEnd - lineEnd - 2);
	if (!parseHeaders(rawHeaders, _headers))
		return false;

	std::string rawBody = rawRequest.substr(headerEnd + 4);

	parseBody(rawBody, _headers, _body);

	return validate();
}

bool HTTPRequest::validate() const {
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return false;

	if (_path.empty() || _path[0] != '/')
		return false;

	if (_protocol != "HTTP")
		return false;

	if (_version != "1.0" && _version != "1.1")
		return false;

	if (getHeader("Host").empty())
		return false;

	if (_method == "POST") {
		bool hasLength = !getHeader("Content-Length").empty();
		bool isChunked = safeToLower(getHeader("Transfer-Encoding")) == "chunked";
		if (!hasLength && !isChunked)
			return false;
	}

	return true;
}

void HTTPRequest::fill1() {
	_method = "GET";
	_path = "/test";
	_protocol = "HTTP";
	_version = "1.1";
	_queries["name"] = "value";
	_headers["Content-Type"] = "text/html";
	_headers["Host"] = "localhost";
}

void HTTPRequest::fill2() {
	_method = "GET";
	_path = "/cgi-index";
	_protocol = "HTTP";
	_version = "1.1";
	_queries["name"] = "value";
	_headers["Content-Type"] = "text/html";
	_headers["Host"] = "localhost";
}
