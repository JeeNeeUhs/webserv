#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

class HTTPRequest {
	private:
		std::string _method;
		std::string _path;
		std::string _query;
		std::string _protocol;
		std::string _version;

		std::map<std::string, std::string> _queries;
		std::map<std::string, std::string> _headers;

		std::string _body;

		void parseQueryString(const std::string& queryStr);
		bool validate(void) const;

	public:
		HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		~HTTPRequest();

		const std::string& getMethod(void) const;
		const std::string& getPath(void) const;
		const std::string& getQuery(void) const;
		const std::string& getProtocol(void) const;
		const std::string& getVersion(void) const;
		const std::map<std::string, std::string>& getQueries(void) const;
		const std::map<std::string, std::string>& getHeaders(void) const;
		const std::vector<std::string>& getHeaderValuesList(const std::string& key) const;
		const std::string& getBody(void) const;
		std::string getHeader(const std::string& key) const;
		std::string getUnparsedRequest(void) const;

		bool parse(const std::string& rawRequest, size_t headerEnd);
};

/*
POST /index HTTP/1.0
Content-Type: text/html
Content-Length: 100
User-Agent: curl/8.4.0
Host: www.example.com
User-Agent: Mozilla/5.0 (X11; Linux x86_64)
Accept: text/html, application/xhtml+xml, application/xml;q=0.9, *//*;q=0.8
Accept-Language: tr-TR, tr;q=0.9, en-US;q=0.8, en;q=0.7
Accept-Charset: utf-8, iso-8859-9;q=0.9, *;q=0.8
Accept-Encoding: gzip, compress
Referer: http://www.google.com/
Authorization: Basic ZW1pcmhhbjpwYXNzd29yZA==

bodydeneme
*/

#endif
