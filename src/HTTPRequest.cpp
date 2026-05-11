#include "HTTPRequest.hpp"
#include <cstdlib>
#include <cctype>

// --- Static helpers ---

static std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

static std::string toLower(const std::string& s) {
	std::string result = s;
	for (size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
	return result;
}

static int hexToInt(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static std::string urlDecode(const std::string& encoded) {
	std::string decoded;
	decoded.reserve(encoded.size());
	for (size_t i = 0; i < encoded.size(); ++i) {
		if (encoded[i] == '%' && i + 2 < encoded.size()) {
			int hi = hexToInt(encoded[i + 1]);
			int lo = hexToInt(encoded[i + 2]);
			if (hi >= 0 && lo >= 0) {
				decoded += static_cast<char>(hi * 16 + lo);
				i += 2;
				continue;
			}
		} else if (encoded[i] == '+') {
			decoded += ' ';
			continue;
		}
		decoded += encoded[i];
	}
	return decoded;
}

// --- HTTPRequest ---

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

const std::string& HTTPRequest::getMethod() const   { return _method; }
const std::string& HTTPRequest::getPath() const     { return _path; }
const std::string& HTTPRequest::getProtocol() const { return _protocol; }
const std::string& HTTPRequest::getVersion() const  { return _version; }

const std::map<std::string, std::string>& HTTPRequest::getQueries() const {
	return _queries;
}

const std::map<std::string, std::string>& HTTPRequest::getHeaders() const {
	return _headers;
}

const std::string& HTTPRequest::getBody() const {
	return _body;
}

// Case-insensitive header lookup
std::string HTTPRequest::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;

	std::string lowerKey = toLower(key);
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		if (toLower(it->first) == lowerKey)
			return it->second;
	}
	return "";
}

// Parse key=value&key2=value2 pairs into _queries
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

// Decode chunked transfer-encoding body
std::string HTTPRequest::_parseChunkedBody(const std::string& raw) const {
	std::string body;
	size_t pos = 0;

	while (pos < raw.size()) {
		size_t lineEnd = raw.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			break;

		std::string sizeStr = raw.substr(pos, lineEnd - pos);
		// Strip optional chunk extensions (";ext=val")
		size_t semiPos = sizeStr.find(';');
		if (semiPos != std::string::npos)
			sizeStr = sizeStr.substr(0, semiPos);

		char* endPtr;
		long chunkSize = std::strtol(sizeStr.c_str(), &endPtr, 16);
		if (chunkSize == 0)
			break; // final zero-length chunk
		if (chunkSize < 0)
			return body;

		pos = lineEnd + 2;
		size_t uChunkSize = static_cast<size_t>(chunkSize);
		if (pos + uChunkSize > raw.size())
			break; // incomplete data

		body.append(raw, pos, uChunkSize);
		pos += uChunkSize + 2; // skip chunk data + trailing \r\n
	}
	return body;
}

bool HTTPRequest::parse(const std::string& rawRequest) {
	_method.clear();
	_path.clear();
	_protocol.clear();
	_version.clear();
	_queries.clear();
	_headers.clear();
	_body.clear();

	// Headers section ends at the first blank line (\r\n\r\n)
	size_t headerEnd = rawRequest.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	// --- Request line ---
	size_t lineEnd = rawRequest.find("\r\n");
	if (lineEnd == std::string::npos || lineEnd > headerEnd)
		return false;

	std::string requestLine = rawRequest.substr(0, lineEnd);

	size_t firstSpace = requestLine.find(' ');
	if (firstSpace == std::string::npos)
		return false;

	size_t secondSpace = requestLine.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
		return false;

	_method = requestLine.substr(0, firstSpace);
	std::string fullPath = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	std::string versionStr = trim(requestLine.substr(secondSpace + 1));

	// Split path from query string
	size_t queryPos = fullPath.find('?');
	if (queryPos != std::string::npos) {
		_path = urlDecode(fullPath.substr(0, queryPos));
		_parseQueryString(fullPath.substr(queryPos + 1));
	} else {
		_path = urlDecode(fullPath);
	}

	// Parse "HTTP/1.1" → protocol="HTTP", version="1.1"
	size_t slashPos = versionStr.find('/');
	if (slashPos == std::string::npos)
		return false;

	_protocol = versionStr.substr(0, slashPos);
	_version = versionStr.substr(slashPos + 1);

	// --- Headers ---
	size_t pos = lineEnd + 2;
	while (pos < headerEnd) {
		size_t nextLine = rawRequest.find("\r\n", pos);
		if (nextLine == std::string::npos || nextLine > headerEnd)
			break;

		std::string headerLine = rawRequest.substr(pos, nextLine - pos);
		size_t colonPos = headerLine.find(':');
		if (colonPos != std::string::npos) {
			std::string key = trim(headerLine.substr(0, colonPos));
			std::string value = trim(headerLine.substr(colonPos + 1));
			if (!key.empty())
				_headers[key] = value;
		}
		pos = nextLine + 2;
	}

	// --- Body ---
	size_t bodyStart = headerEnd + 4;

	std::string te = getHeader("Transfer-Encoding");
	if (toLower(te) == "chunked") {
		if (bodyStart < rawRequest.size())
			_body = _parseChunkedBody(rawRequest.substr(bodyStart));
	} else {
		std::string clStr = getHeader("Content-Length");
		if (!clStr.empty()) {
			long contentLength = std::strtol(clStr.c_str(), NULL, 10);
			if (contentLength > 0 && bodyStart < rawRequest.size())
				_body = rawRequest.substr(bodyStart, static_cast<size_t>(contentLength));
		}
	}

	return validate();
}

bool HTTPRequest::validate() const {
	// Method must be a supported verb
	if (_method != "GET" && _method != "POST" && _method != "DELETE" &&
		_method != "HEAD" && _method != "PUT" && _method != "OPTIONS")
		return false;

	// Path must be non-empty and absolute
	if (_path.empty() || _path[0] != '/')
		return false;

	// Protocol must be HTTP
	if (_protocol != "HTTP")
		return false;

	// Only HTTP/1.0 and HTTP/1.1 are supported
	if (_version != "1.0" && _version != "1.1")
		return false;

	// HTTP/1.1 requires a Host header
	if (_version == "1.1" && getHeader("Host").empty())
		return false;

	// POST must declare a body length
	if (_method == "POST") {
		bool hasLength = !getHeader("Content-Length").empty();
		bool isChunked = toLower(getHeader("Transfer-Encoding")) == "chunked";
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
