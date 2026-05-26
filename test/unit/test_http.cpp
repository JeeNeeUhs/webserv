#include "HTTPParser.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "webserv.hpp"

#include <iostream>

template <typename T>
static bool assert_equal(const T& actual, const T& expected) {
	if (actual != expected) {
		std::cerr << "fail: "
			<< " (expected: '" << expected
			<< "', actual: '" << actual << "')"
			<< std::endl;
		return false;
	}

	return true;
}

static bool test_request_get(void) {
	std::string raw = "GET /index.html?query=string HTTP/1.1\r\nUser-Agent: curl\r\n\r\n";
	size_t headerEnd = raw.find("\r\n\r\n") + 4;

	HTTPRequest req;
	bool parsed = req.parse(raw, headerEnd);

	if (!assert_equal(parsed, true))
		return false;
	if (!assert_equal(req.getMethod(), std::string("GET")))
		return false;
	if (!assert_equal(req.getPath(), std::string("/index.html")))
		return false;
	if (!assert_equal(req.getProtocol(), std::string("HTTP")))
		return false;
	if (!assert_equal(req.getVersion(), std::string("1.1")))
		return false;
	if (!assert_equal(req.getHeader("user-agent"), std::string("curl")))
		return false;

	return true;
}

static bool test_request_url_decode(void) {
	std::string raw = "GET /filewith%20space?query=string+midir&query2=bosluk%20var HTTP/1.1\r\n\r\n";
	size_t headerEnd = raw.find("\r\n\r\n") + 4;

	HTTPRequest req;
	bool parsed = req.parse(raw, headerEnd);

	if (!assert_equal(parsed, true))
		return false;
	if (!assert_equal(req.getPath(), std::string("/filewith space")))
		return false;

	std::map<std::string, std::string> queries = req.getQueries();
	if (!assert_equal(queries["query"], std::string("string midir")))
		return false;
	if (!assert_equal(queries["query2"], std::string("bosluk var")))
		return false;

	return true;
}

static bool test_request_validation_fail(void) {
	// nonexist Content-Length or Transfer-Encoding header
	std::string raw = "POST /submit HTTP/1.1\r\nContent-Type: text/plain\r\n\r\npatlicanki";
	size_t headerEnd = raw.find("\r\n\r\n") + 4;

	HTTPRequest req;
	bool parsed = req.parse(raw, headerEnd);

	if (!assert_equal(parsed, false))
		return false;
	return true;
}

static bool test_chunked_valid(void) {
	std::string req = "5\r\nnaber\r\n5\r\nkanka\r\n0\r\n\r\n";
	std::string body;

	bool success = HTTPParser::parseChunkedBody(req, body);

	if (!assert_equal(success, true))
		return false;
	if (!assert_equal(body, std::string("naberkanka")))
		return false;

	return true;
}

static bool test_chunked_incomplete(void) {
	std::string raw = "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n";
	size_t headerEnd = raw.size();

	raw += "3\r\nnab\r\n2\r\ner";
	
	HTTPParser::RequestStatus status = HTTPParser::checkComplete(raw, headerEnd);
	
	if (!assert_equal(status, HTTPParser::REQ_INCOMPLETE))
		return false;
	return true;
}

static bool test_chunked_bad_size(void) {
	std::string raw = "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n";
	size_t headerEnd = raw.size();
	
	raw += "XYZ\r\nbozukmuyumneyimben\r\n";

	HTTPParser::RequestStatus status = HTTPParser::checkComplete(raw, headerEnd);

	if (!assert_equal(status, HTTPParser::REQ_BAD))
		return false;
	return true;
}

static bool test_response_serialize(void) {
	HTTPResponse res;

	res.setStatusCode(404);
	res.addHeader("Connection", "close");
	res.setBody("page not found babus");

	std::string output = res.serialize();

	std::string expected =
		"HTTP/" + std::string(HTTP_VERSION) + " 404 Not Found\r\n"
		"Connection: close\r\n"
		"Content-Length: 20\r\n"
		"\r\n"
		"page not found babus";

	if (!assert_equal(output, expected))
		return false;
	return true;
}

size_t run_http_tests(void) {
	size_t failed = 0;
	
	std::cout << "running http tests..." << std::endl;

	if (!test_request_get())
		failed++;
	if (!test_request_url_decode())
		failed++;
	if (!test_request_validation_fail())
		failed++;
	if (!test_chunked_valid())
		failed++;
	if (!test_chunked_incomplete())
		failed++;
	if (!test_chunked_bad_size())
		failed++;
	if (!test_response_serialize())
		failed++;

	return failed;
}
