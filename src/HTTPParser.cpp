#include "HTTPRequest.hpp"
#include <cstdlib>
#include <cctype>
#include <map>

std::string httpLineTrim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t");

	if (start == std::string::npos)
		return "";

	size_t end = s.find_last_not_of(" \t\r\n");

	return s.substr(start, end - start + 1);
}

std::string safeToLower(const std::string& s) {
	std::string result = s;

	for (size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));

	return result;
}

int hexToInt(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

std::string urlDecode(const std::string& encoded) {
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

std::string parseChunkedBody(const std::string& raw) {
	std::string body;
	size_t pos = 0;

	while (pos < raw.size()) {
		size_t lineEnd = raw.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			break;

		std::string sizeStr = raw.substr(pos, lineEnd - pos);
		size_t semiPos = sizeStr.find(';');
		if (semiPos != std::string::npos)
			sizeStr = sizeStr.substr(0, semiPos);

		char* endPtr;
		long chunkSize = std::strtol(sizeStr.c_str(), &endPtr, 16);
		if (chunkSize == 0)
			break;
		if (chunkSize < 0)
			return body;

		pos = lineEnd + 2;
		size_t uChunkSize = static_cast<size_t>(chunkSize);
		if (pos + uChunkSize > raw.size())
			break;

		body.append(raw, pos, uChunkSize);
		pos += uChunkSize + 2;
	}
	return body;
}

bool parseRequestLine(const std::string& line, std::string& method, std::string& path, std::string& protocol, std::string& version) {
	size_t firstSpace = line.find(' ');
	if (firstSpace == std::string::npos)
		return false;

	size_t secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
		return false;

	method = line.substr(0, firstSpace);
	std::string fullPath = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	std::string versionStr = httpLineTrim(line.substr(secondSpace + 1));

	size_t queryPos = fullPath.find('?');
	if (queryPos != std::string::npos)
		path = urlDecode(fullPath.substr(0, queryPos));
	else
		path = urlDecode(fullPath);


	size_t slashPos = versionStr.find('/');
	if (slashPos == std::string::npos)
		return false;

	protocol = versionStr.substr(0, slashPos);
	version = versionStr.substr(slashPos + 1);

	return true;
}

bool parseHeaders(const std::string& rawHeaders, std::map<std::string, std::string>& headers) {
	size_t pos = 0;

	while (pos < rawHeaders.size()) {
		size_t nextLine = rawHeaders.find("\r\n", pos);
		if (nextLine == std::string::npos || nextLine > rawHeaders.size())
			break;

		std::string headerLine = rawHeaders.substr(pos, nextLine - pos);
		size_t colonPos = headerLine.find(':');
		if (colonPos != std::string::npos) {
			std::string key = httpLineTrim(headerLine.substr(0, colonPos));
			std::string value = httpLineTrim(headerLine.substr(colonPos + 1));
			if (!key.empty())
				headers[key] = value;
			else
				return false;
		} else {
			return false;
		}
		pos = nextLine + 2;
	}

	return true;
}

void parseBody(const std::string& rawBody, const std::map<std::string, std::string>& headers, std::string& body) {
	size_t bodyStart = 0;

	std::string te = headers.find("Transfer-Encoding") != headers.end() ? headers.at("Transfer-Encoding") : "";
	if (safeToLower(te) == "chunked") {
		if (bodyStart < rawBody.size())
			body = parseChunkedBody(rawBody.substr(bodyStart));
	} else {
		std::string clStr = headers.find("Content-Length") != headers.end() ? headers.at("Content-Length") : "";
		if (!clStr.empty()) {
			long contentLength = std::strtol(clStr.c_str(), NULL, 10);
			if (contentLength > 0 && bodyStart < rawBody.size())
				body = rawBody.substr(bodyStart, static_cast<size_t>(contentLength));
		}
	}
}
