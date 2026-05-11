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

class HTTPResponse {};

#endif
