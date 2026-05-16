#include "HTTPRequest.hpp"
#include <iostream>
#include <string>

static int passed = 0;
static int failed = 0;

static void check(const std::string& name, bool result) {
	if (result) {
		std::cout << "[PASS] " << name << "\n";
		passed++;
	} else {
		std::cout << "[FAIL] " << name << "\n";
		failed++;
	}
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

static bool parses(const std::string& raw) {
	HTTPRequest req;
	return req.parse(raw);
}

// ─── Tests ───────────────────────────────────────────────────────────────────

static void testGetBasic() {
	std::string raw =
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";
	HTTPRequest req;
	bool ok = req.parse(raw);
	check("GET basic parses", ok);
	check("GET method", req.getMethod() == "GET");
	check("GET path", req.getPath() == "/index.html");
	check("GET protocol", req.getProtocol() == "HTTP");
	check("GET version", req.getVersion() == "1.1");
	check("GET Host header", req.getHeader("Host") == "localhost");
	check("GET body empty", req.getBody().empty());
}

static void testPostWithBody() {
	std::string raw =
		"POST /submit HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"name=John+Doe";
	HTTPRequest req;
	bool ok = req.parse(raw);
	check("POST parses", ok);
	check("POST method", req.getMethod() == "POST");
	check("POST path", req.getPath() == "/submit");
	check("POST body", req.getBody() == "name=John+Doe");
}

static void testDeleteBasic() {
	std::string raw =
		"DELETE /resource/42 HTTP/1.1\r\n"
		"Host: api.example.com\r\n"
		"\r\n";
	HTTPRequest req;
	bool ok = req.parse(raw);
	check("DELETE parses", ok);
	check("DELETE method", req.getMethod() == "DELETE");
	check("DELETE path", req.getPath() == "/resource/42");
}

static void testMethodRejections() {
	check("PUT rejected",
		!parses("PUT /res HTTP/1.1\r\nHost: x\r\n\r\n"));
	check("HEAD rejected",
		!parses("HEAD / HTTP/1.1\r\nHost: x\r\n\r\n"));
	check("OPTIONS rejected",
		!parses("OPTIONS / HTTP/1.1\r\nHost: x\r\n\r\n"));
	check("PATCH rejected",
		!parses("PATCH / HTTP/1.1\r\nHost: x\r\n\r\n"));
}

static void testHttp10NoHostRequired() {
	std::string raw =
		"GET / HTTP/1.0\r\n"
		"\r\n";
	check("HTTP/1.0 no Host accepted", parses(raw));
}

static void testHttp11RequiresHost() {
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"\r\n";
	check("HTTP/1.1 without Host rejected", !parses(raw));
}

static void testPostRequiresContentLength() {
	std::string raw =
		"POST /data HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";
	check("POST without Content-Length rejected", !parses(raw));
}

static void testPostChunked() {
	std::string raw =
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"5\r\n"
		"hello\r\n"
		"6\r\n"
		" world\r\n"
		"0\r\n"
		"\r\n";
	HTTPRequest req;
	bool ok = req.parse(raw);
	check("POST chunked parses", ok);
	check("POST chunked body decoded", req.getBody() == "hello world");
}

static void testQueryStringParsing() {
	std::string raw =
		"GET /search?q=hello+world&page=2 HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";
	HTTPRequest req;
	req.parse(raw);
	check("Query path stripped", req.getPath() == "/search");
	check("Query q value", req.getQueries().count("q") && req.getQueries().at("q") == "hello world");
	check("Query page value", req.getQueries().count("page") && req.getQueries().at("page") == "2");
}

static void testUrlDecoding() {
	std::string raw =
		"GET /path%2Fwith%20spaces HTTP/1.1\r\n"
		"Host: x\r\n"
		"\r\n";
	HTTPRequest req;
	req.parse(raw);
	check("URL decode %2F and %20", req.getPath() == "/path/with spaces");
}

static void testInvalidProtocol() {
	check("Non-HTTP protocol rejected",
		!parses("GET / FTP/1.1\r\nHost: x\r\n\r\n"));
}

static void testUnsupportedVersion() {
	check("HTTP/2.0 rejected",
		!parses("GET / HTTP/2.0\r\nHost: x\r\n\r\n"));
	check("HTTP/0.9 rejected",
		!parses("GET / HTTP/0.9\r\nHost: x\r\n\r\n"));
}

static void testEmptyPath() {
	check("Empty path rejected",
		!parses("GET  HTTP/1.1\r\nHost: x\r\n\r\n"));
}

static void testRelativePath() {
	check("Relative path rejected",
		!parses("GET relative/path HTTP/1.1\r\nHost: x\r\n\r\n"));
}

static void testHeaderCaseInsensitive() {
	std::string raw =
		"GET / HTTP/1.1\r\n"
		"host: example.com\r\n"
		"content-type: text/plain\r\n"
		"\r\n";
	HTTPRequest req;
	req.parse(raw);
	check("Header lookup case-insensitive (host)", req.getHeader("Host") == "example.com");
	check("Header lookup case-insensitive (Content-Type)", req.getHeader("content-type") == "text/plain");
}

static void testMalformedRequest() {
	check("No CRLF CRLF rejected", !parses("GET / HTTP/1.1\r\nHost: x\r\n"));
	check("No request line rejected", !parses("\r\n\r\n"));
}

static void testCopyAndAssign() {
	std::string raw =
		"GET /copy HTTP/1.1\r\n"
		"Host: copy.test\r\n"
		"\r\n";
	HTTPRequest a;
	a.parse(raw);

	HTTPRequest b(a);
	check("Copy constructor method", b.getMethod() == "GET");
	check("Copy constructor path", b.getPath() == "/copy");

	HTTPRequest c;
	c = a;
	check("Assignment operator method", c.getMethod() == "GET");
	check("Assignment operator path", c.getPath() == "/copy");
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main() {
	testGetBasic();
	testPostWithBody();
	testDeleteBasic();
	testMethodRejections();
	testHttp10NoHostRequired();
	testHttp11RequiresHost();
	testPostRequiresContentLength();
	testPostChunked();
	testQueryStringParsing();
	testUrlDecoding();
	testInvalidProtocol();
	testUnsupportedVersion();
	testEmptyPath();
	testRelativePath();
	testHeaderCaseInsensitive();
	testMalformedRequest();
	testCopyAndAssign();

	std::cout << "\n" << passed << " passed, " << failed << " failed\n";
	return (failed == 0) ? 0 : 1;
}
