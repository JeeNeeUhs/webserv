#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <map>

class HTTPRequest {
	private:
		std::string _method;
		std::string _path;
		std::string _protocol;
		std::string _version;

		std::map<std::string, std::string> _queries;
		std::map<std::string, std::string> _headers;

		std::string _body;

		void _parseQueryString(const std::string& queryStr);
		std::string _parseChunkedBody(const std::string& raw) const;

	public:
		HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		~HTTPRequest();

		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getProtocol() const;
		const std::string& getVersion() const;
		const std::map<std::string, std::string>& getQueries() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		std::string getHeader(const std::string& key) const;

		void fill1();
		void fill2();

		bool validate() const;
		bool parse(const std::string& rawRequest);
};

std::string httpLineTrim(const std::string& s);
std::string safeToLower(const std::string& s);
int hexToInt(char c);
std::string urlDecode(const std::string& encoded);
bool parseRequestLine(const std::string& line, std::string& method, std::string& path, std::string& protocol, std::string& version);
bool parseHeaders(const std::string& rawHeaders, std::map<std::string, std::string>& headers);
void parseBody(const std::string& rawBody, const std::map<std::string, std::string>& headers, std::string& body);

#endif
