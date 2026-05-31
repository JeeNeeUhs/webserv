#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include <map>

class HTTPResponse {
	private:
		std::string _version;
		size_t _statusCode;
		std::string _reasonPhrase;
		std::map<std::string, std::string> _headers;
		std::string _body;

		bool _isFileBody;
		std::string _filePath;
		size_t _fileSize;

	public:
		HTTPResponse();
		HTTPResponse(const HTTPResponse& other);
		HTTPResponse& operator=(const HTTPResponse& other);
		~HTTPResponse();

		size_t getStatusCode(void) const;
		bool isFileBody(void) const;
		const std::string& getFilePath(void) const;
		size_t getFileSize(void) const;

		void setStatusCode(int statusCode);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);
		void setFileBody(const std::string& path, size_t size);

		std::string serialize(void);
};

/*
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

#endif
