#include "HTTPParser.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cctype>
#include <limits>

static std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";

	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

static int hexToInt(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return -1;
}

static bool isChunked(const std::string& transferEncoding) {
	return HTTPParser::toLower(transferEncoding).find("chunked") != std::string::npos;
}

static HTTPParser::RequestStatus chunkedStatus(const std::string& buffer) {
	size_t pos = 0;

	while (pos < buffer.size()) {
		size_t lineEnd = buffer.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			return HTTPParser::REQ_INCOMPLETE;

		std::string sizeStr = buffer.substr(pos, lineEnd - pos);
		size_t semi = sizeStr.find(';');
		if (semi != std::string::npos)
			sizeStr = sizeStr.substr(0, semi);

		char* endPtr = NULL;
		long chunkSize = std::strtol(sizeStr.c_str(), &endPtr, 16);
		if (endPtr == sizeStr.c_str() || chunkSize < 0)
			return HTTPParser::REQ_BAD;
		if (chunkSize == 0)
			return HTTPParser::REQ_COMPLETE;

		pos = lineEnd + 2;
		size_t need = static_cast<size_t>(chunkSize) + 2;
		if (pos + need > buffer.size())
			return HTTPParser::REQ_INCOMPLETE;

		pos += need;
	}

	return HTTPParser::REQ_INCOMPLETE;
}

std::string HTTPParser::toLower(const std::string& s) {
	std::string result = s;
	for (size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));

	return result;
}

std::string HTTPParser::urlDecode(const std::string& encoded) {
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

std::string HTTPParser::peekHeader(const std::string& rawHeaders, const std::string& name) {
	std::string lower = HTTPParser::toLower(rawHeaders);
	std::string key = HTTPParser::toLower(name) + ":";

	size_t pos = lower.find(key);
	if (pos == std::string::npos)
		return "";

	size_t valStart = pos + key.size();
	size_t eol = rawHeaders.find("\r\n", valStart);
	std::string raw = rawHeaders.substr(valStart,
		(eol == std::string::npos ? rawHeaders.size() : eol) - valStart);

	return trim(raw);
}

size_t HTTPParser::findHeaderEnd(const std::string& buffer) {
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return std::string::npos;

	return pos + 4;
}

HTTPParser::RequestStatus HTTPParser::checkComplete(Connection& c) {
	std::string te = c.req.getHeader("Transfer-Encoding");
	if (isChunked(te))
		return chunkedStatus(c.readBuff);

	if (c.contentLength == static_cast<size_t>(-1)) {
		std::string cl = c.req.getHeader("Content-Length");
		if (!cl.empty()) {
			try {
				c.contentLength = utils::parseNum<size_t>(cl);
			} catch (...) {
				return REQ_BAD;
			}
		}
		c.cgiBytesWritten = 0;
	}

	if (c.contentLength != static_cast<size_t>(-1)) {
		c.cgiBytesWritten += c.readBuff.size();
		c.cgiReadBuff.append(c.readBuff);
		c.readBuff.clear();
		if (c.cgiBytesWritten > c.config->clientMaxBodySize)
			return REQ_TOO_LARGE;
		else if (c.cgiBytesWritten < c.contentLength)
			return REQ_INCOMPLETE;
		else if (c.cgiBytesWritten > c.contentLength)
			return REQ_BAD;
	}

	return REQ_COMPLETE;
}

static bool parseChunkSize(const std::string& s, std::size_t& out)
{
	if (s.empty())
		return false;
	std::size_t result = 0;
	for (std::size_t i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		int digit;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else
			return false;
		if (result > (std::numeric_limits<std::size_t>::max() - digit) / 16)
			return false;
		result = result * 16 + digit;
	}
	out = result;
	return true;
}

HTTPParser::RequestStatus HTTPParser::parseChunkedBody(std::string& raw, std::string& body) {
	std::size_t pos = 0;

	while (true) {
		std::size_t lineEnd = raw.find("\r\n", pos);
		if (lineEnd == std::string::npos) {
			raw.erase(0, pos);
			return REQ_INCOMPLETE;
		}

		std::string sizeLine = raw.substr(pos, lineEnd - pos);
		std::size_t semicolon = sizeLine.find(';');
		std::string hexStr = (semicolon == std::string::npos) ? sizeLine : sizeLine.substr(0, semicolon);

		std::size_t chunkSize;
		if (!parseChunkSize(hexStr, chunkSize))
			return REQ_BAD;

		if (chunkSize == 0) {
			std::size_t tpos = lineEnd + 2;
			while (true) {
				std::size_t tEnd = raw.find("\r\n", tpos);
				if (tEnd == std::string::npos) {
					raw.erase(0, pos);
					return REQ_INCOMPLETE;
				}
				if (tEnd == tpos) {
					raw.erase(0, tEnd + 2);
					return REQ_COMPLETE;
				}
				tpos = tEnd + 2;
			}
		}

		std::size_t dataStart = lineEnd + 2;
		if (raw.size() < dataStart + chunkSize + 2) {
			raw.erase(0, pos);
			return REQ_INCOMPLETE;
		}

		if (raw[dataStart + chunkSize] != '\r' || raw[dataStart + chunkSize + 1] != '\n')
			return REQ_BAD;
		
		body.append(raw, dataStart, chunkSize);
		pos = dataStart + chunkSize + 2;
	}
}

bool HTTPParser::parseRequestLine(const std::string& line, std::string& method,
		std::string& query, std::string& path, std::string& protocol, std::string& version) {
	size_t firstSpace = line.find(' ');
	if (firstSpace == std::string::npos || firstSpace == 0)
		return false;

	size_t secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos || secondSpace == firstSpace + 1)
		return false;

	method = line.substr(0, firstSpace);
	std::string fullPath = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	std::string versionStr = trim(line.substr(secondSpace + 1));

	size_t queryPos = fullPath.find('?');
	if (queryPos != std::string::npos) {
		path = urlDecode(fullPath.substr(0, queryPos));
		query = fullPath.substr(queryPos + 1);
	} else {
		path = urlDecode(fullPath);
		query = "";
	}

	size_t slashPos = versionStr.find('/');
	if (slashPos == std::string::npos)
		return false;

	protocol = versionStr.substr(0, slashPos);
	version = versionStr.substr(slashPos + 1);

	return true;
}

bool HTTPParser::parseHeaders(const std::string& rawHeaders, std::map<std::string, std::string>& headers) {
	size_t pos = 0;

	while (pos < rawHeaders.size()) {
		size_t nextLine = rawHeaders.find("\r\n", pos);
		if (nextLine == std::string::npos)
			break;

		std::string headerLine = rawHeaders.substr(pos, nextLine - pos);
		size_t colonPos = headerLine.find(':');
		if (colonPos == std::string::npos)
			return false;

		std::string key = trim(headerLine.substr(0, colonPos));
		std::string value = trim(headerLine.substr(colonPos + 1));
		if (key.empty())
			return false;

		headers[key] = value;
		pos = nextLine + 2;
	}

	return true;
}

bool HTTPParser::parseBody(std::string& rawBody, const std::map<std::string, std::string>& headers, std::string& body) {
	std::string te = (headers.find("Transfer-Encoding") != headers.end())
		? headers.find("Transfer-Encoding")->second : "";

	if (isChunked(te))
		return parseChunkedBody(rawBody, body);

	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
	if (it != headers.end()) {
		std::size_t len = 0;

		try {
			len = utils::parseNum<std::size_t>(it->second);
		} catch (...) {
			return false;
		}

		body = (len <= rawBody.size()) ? rawBody.substr(0, len) : rawBody;
	}

	return true;
}
