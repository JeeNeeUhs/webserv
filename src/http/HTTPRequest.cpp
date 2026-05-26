#include "HTTPRequest.hpp"
#include "HTTPParser.hpp"
#include "utils.hpp"

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const HTTPRequest& other) {
	operator=(other);
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) {
	if (this != &other) {
		_method = other._method;
		_path = other._path;
		_query = other._query;
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

const std::string& HTTPRequest::getQuery() const {
	return _query;
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

	std::string lowerKey = HTTPParser::toLower(key);
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		if (HTTPParser::toLower(it->first) == lowerKey)
			return it->second;
	}

	return "";
}

void HTTPRequest::parseQueryString(const std::string& queryStr) {
	size_t pos = 0;

	while (pos <= queryStr.size()) {
		size_t ampPos = queryStr.find('&', pos);
		if (ampPos == std::string::npos)
			ampPos = queryStr.size();

		std::string pair = queryStr.substr(pos, ampPos - pos);
		if (!pair.empty()) {
			size_t eqPos = pair.find('=');
			if (eqPos != std::string::npos)
				_queries[HTTPParser::urlDecode(pair.substr(0, eqPos))] = HTTPParser::urlDecode(pair.substr(eqPos + 1));
			else
				_queries[HTTPParser::urlDecode(pair)] = "";
		}
		if (ampPos == queryStr.size())
			break;
		pos = ampPos + 1;
	}
}

bool HTTPRequest::parse(const std::string& rawRequest, size_t headerEnd) {
	_method.clear();
	_path.clear();
	_query.clear();
	_protocol.clear();
	_version.clear();
	_queries.clear();
	_headers.clear();
	_body.clear();

	// headerEnd "\r\n\r\n" + 4
	if (headerEnd < 4 || headerEnd > rawRequest.size())
		return false;

	size_t headerBlockEnd = headerEnd - 4;
	size_t lineEnd = rawRequest.find("\r\n");

	if (lineEnd == std::string::npos || lineEnd > headerBlockEnd)
		return false;

	if (!HTTPParser::parseRequestLine(rawRequest.substr(0, lineEnd),
		_method, _query, _path, _protocol, _version))
		return false;

	std::string rawHeaders = rawRequest.substr(lineEnd + 2, headerBlockEnd - (lineEnd + 2) + 2);
	if (!HTTPParser::parseHeaders(rawHeaders, _headers))
		return false;

	if (!_query.empty())
		parseQueryString(_query);

	std::string rawBody = rawRequest.substr(headerEnd);
	if (!HTTPParser::parseBody(rawBody, _headers, _body))
		return false;

	return validate();
}

bool HTTPRequest::validate(void) const {
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return false;

	if (_path.empty() || _path[0] != '/')
		return false;

	if (_protocol != "HTTP")
		return false;

	if (_version != "1.0" && _version != "1.1")
		return false;

	// if (getHeader("Host").empty())
		// return false;

	std::string clHeader = getHeader("Content-Length");
	bool isChunked = HTTPParser::toLower(getHeader("Transfer-Encoding")) == "chunked";
	bool hasLength = !clHeader.empty();

	if (hasLength && isChunked)
		return false;

	if (_method == "POST") {
		if (!hasLength && !isChunked)
			return false;
	}

	return true;
}
