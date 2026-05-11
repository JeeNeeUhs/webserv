#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>

/*
Response:

HTTP/1.0 500 abc def
Date: Sun, 05 Apr 2026 12:00:00 GMT
Server: webserv/1.0
Content-Type: text/html; charset=UTF-8
Content-Length: 348
Content-Language: tr
Content-Encoding: gzip
Last-Modified: Sat, 04 Apr 2026 18:30:00 GMT
Expires: Mon, 06 Apr 2026 12:00:00 GMT
Cache-Control: public, max-age=86400
Pragma: no-cache
Vary: Accept-Encoding, Accept-Language
Location: /redirected-page
Allow: GET, POST, HEAD
Connection: close

<html>default error page</html>
*/

class HTTPResponse {
	private:
		std::string _version;
		int _statusCode;
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		HTTPResponse();
		HTTPResponse(int statusCode, const std::string& reasonPhrase);
		HTTPResponse(int statusCode, const std::string& reasonPhrase, const std::map<std::string, std::string>& headers, const std::string& body);
		HTTPResponse(const HTTPResponse& other);
		HTTPResponse& operator=(const HTTPResponse& other);
		~HTTPResponse();

		void setStatusCode(int statusCode);
		void setReasonPhrase(const std::string& reasonPhrase);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);

		const std::string toString() const;
};

#endif
