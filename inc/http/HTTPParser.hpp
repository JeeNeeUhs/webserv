#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include <string>
# include <map>

namespace HTTPParser {
	enum RequestStatus {
		REQ_COMPLETE,
		REQ_INCOMPLETE,
		REQ_BAD
	};

	std::string toLower(const std::string& s);
	std::string urlDecode(const std::string& encoded);

	bool parseChunkedBody(const std::string& raw, std::string& body);
	bool parseRequestLine(const std::string& line, std::string& method,
		std::string& query, std::string& path, std::string& protocol, std::string& version);
	bool parseHeaders(const std::string& rawHeaders, std::map<std::string, std::string>& headers);
	bool parseBody(const std::string& rawBody, const std::map<std::string, std::string>& headers, std::string& body);

	// shared with ServerManager
	size_t findHeaderEnd(const std::string& buffer);
	RequestStatus checkComplete(const std::string& buffer, std::size_t headerEnd);
}

#endif