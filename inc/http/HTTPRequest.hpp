#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <map>

/*
Request:

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

class HTTPRequest {
	private:
		std::string method;
		std::string path;
		std::string protocol;
		std::string version;
		
		std::map<std::string, std::string> headers;

		std::string body;

	public:
		HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		~HTTPRequest();

		std::string getMethod() const;
		std::string getPath() const;
		std::string getProtocol() const;
		std::string getVersion() const;

		std::map<std::string, std::string> getHeaders() const;
		
		std::string getBody() const;

		bool validate();
		bool parse(const std::string& rawRequest);
};

#endif
