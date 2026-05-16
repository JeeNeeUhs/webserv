#include "config/Config.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
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

// Write content to a temp file and return the path
static std::string writeTmp(const std::string& content) {
	char path[] = "/tmp/webserv_test_XXXXXX";
	int fd = mkstemp(path);
	if (fd == -1)
		throw std::runtime_error("mkstemp failed");
	std::string p(path);
	close(fd);
	std::ofstream f(p.c_str());
	f << content;
	return p;
}

static bool throws(const std::string& content) {
	std::string p = writeTmp(content);
	try {
		Config c(p);
		std::remove(p.c_str());
		return false;
	} catch (const std::exception&) {
		std::remove(p.c_str());
		return true;
	}
}

static Config parse(const std::string& content) {
	std::string p = writeTmp(content);
	Config c(p);
	std::remove(p.c_str());
	return c;
}

// ─── Tests ───────────────────────────────────────────────────────────────────

static void testBasicServer() {
	Config c = parse(
		"server {\n"
		"    listen 8080;\n"
		"    location / {\n"
		"        root /var/www;\n"
		"        methods GET;\n"
		"    }\n"
		"}\n"
	);
	check("One server block", c.getServers().size() == 1);
	check("Port 8080", c.getServers()[0].getPort() == 8080);
}

static void testTwoServers() {
	Config c = parse(
		"server { listen 8080; location / { methods GET; } }\n"
		"server { listen 9090; location / { methods GET; } }\n"
	);
	check("Two server blocks", c.getServers().size() == 2);
	check("First port", c.getServers()[0].getPort() == 8080);
	check("Second port", c.getServers()[1].getPort() == 9090);
}

static void testLocationMethods() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    location /upload {\n"
		"        root /var/www;\n"
		"        methods GET POST DELETE;\n"
		"    }\n"
		"}\n"
	);
	const std::vector<std::string>& m = c.getServers()[0].getLocations()[0].getMethods();
	check("Location 3 methods", m.size() == 3);
	check("Location has GET",    m[0] == "GET");
	check("Location has POST",   m[1] == "POST");
	check("Location has DELETE", m[2] == "DELETE");
}

static void testInvalidMethod() {
	check("HEAD rejected in location", throws(
		"server { listen 80; location / { methods HEAD; } }\n"
	));
	check("PUT rejected in location", throws(
		"server { listen 80; location / { methods PUT; } }\n"
	));
	check("HEAD rejected in server", throws(
		"server { listen 80; methods HEAD; location / { methods GET; } }\n"
	));
}

static void testAutoindex() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    location /files {\n"
		"        autoindex on;\n"
		"        methods GET;\n"
		"    }\n"
		"    location /private {\n"
		"        autoindex off;\n"
		"        methods GET;\n"
		"    }\n"
		"}\n"
	);
	const std::vector<Location>& locs = c.getServers()[0].getLocations();
	check("autoindex on",  locs[0].getAutoindex() == true);
	check("autoindex off", locs[1].getAutoindex() == false);
}

static void testClientMaxBodySize() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    client_max_body_size 2M;\n"
		"    location / { methods GET; }\n"
		"}\n"
	);
	check("2M body size", c.getServers()[0].getClientMaxBodySize() == 2 * 1024 * 1024);
}

static void testClientMaxBodySizeK() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    client_max_body_size 512K;\n"
		"    location / { methods GET; }\n"
		"}\n"
	);
	check("512K body size", c.getServers()[0].getClientMaxBodySize() == 512 * 1024);
}

static void testRedirect() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    location /old {\n"
		"        redirect 301 /new;\n"
		"    }\n"
		"}\n"
	);
	const Location& loc = c.getServers()[0].getLocations()[0];
	check("Redirect code 301", loc.getRedirectCode() == 301);
	check("Redirect path /new", loc.getRedirectPath() == "/new");
}

static void testInvalidRedirectCode() {
	check("Redirect 200 rejected", throws(
		"server { listen 80; location /x { redirect 200 /y; } }\n"
	));
	check("Redirect 404 rejected", throws(
		"server { listen 80; location /x { redirect 404 /y; } }\n"
	));
}

static void testErrorPages() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    error_page 404 /errors/404.html;\n"
		"    error_page 500 /errors/500.html;\n"
		"    location / { methods GET; }\n"
		"}\n"
	);
	const std::vector<t_error_page>& ep = c.getServers()[0].getErrorPages();
	check("Two error pages", ep.size() == 2);
	check("404 error page code", ep[0].code == 404);
	check("404 error page path", ep[0].path == "/errors/404.html");
	check("500 error page code", ep[1].code == 500);
}

static void testCgiExtension() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    location /cgi {\n"
		"        cgi_extension .py;\n"
		"        methods GET POST;\n"
		"    }\n"
		"}\n"
	);
	const std::vector<std::string>& exts = c.getServers()[0].getLocations()[0].getCgiExtensions();
	check("CGI extension .py", exts.size() == 1 && exts[0] == ".py");
}

static void testUploadStore() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    location /upload {\n"
		"        upload_store /var/uploads;\n"
		"        methods GET POST;\n"
		"    }\n"
		"}\n"
	);
	check("Upload store path",
		c.getServers()[0].getLocations()[0].getUploadStore() == "/var/uploads");
}

static void testListenWithHost() {
	Config c = parse(
		"server {\n"
		"    listen 127.0.0.1:8080;\n"
		"    location / { methods GET; }\n"
		"}\n"
	);
	check("Listen host:port host", c.getServers()[0].getHost() == "127.0.0.1");
	check("Listen host:port port", c.getServers()[0].getPort() == 8080);
}

static void testMissingListen() {
	check("Missing listen rejected", throws(
		"server { location / { methods GET; } }\n"
	));
}

static void testNoServers() {
	check("Empty config rejected", throws(""));
}

static void testUnknownDirective() {
	check("Unknown server directive rejected", throws(
		"server { listen 80; foobar baz; location / { methods GET; } }\n"
	));
	check("Unknown location directive rejected", throws(
		"server { listen 80; location / { foobar baz; } }\n"
	));
}

static void testCommentIgnored() {
	Config c = parse(
		"# this is a comment\n"
		"server {\n"
		"    listen 8080; # inline comment\n"
		"    location / {\n"
		"        methods GET;\n"
		"    }\n"
		"}\n"
	);
	check("Comments ignored", c.getServers().size() == 1);
}

static void testServerIndex() {
	Config c = parse(
		"server {\n"
		"    listen 80;\n"
		"    index index.html;\n"
		"    location / { methods GET; }\n"
		"}\n"
	);
	check("Server index", c.getServers()[0].getIndex() == "index.html");
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main() {
	try {
		testBasicServer();
		testTwoServers();
		testLocationMethods();
		testInvalidMethod();
		testAutoindex();
		testClientMaxBodySize();
		testClientMaxBodySizeK();
		testRedirect();
		testInvalidRedirectCode();
		testErrorPages();
		testCgiExtension();
		testUploadStore();
		testListenWithHost();
		testMissingListen();
		testNoServers();
		testUnknownDirective();
		testCommentIgnored();
		testServerIndex();
	} catch (const std::exception& e) {
		std::cerr << "Unexpected exception: " << e.what() << "\n";
		return 1;
	}

	std::cout << "\n" << passed << " passed, " << failed << " failed\n";
	return (failed == 0) ? 0 : 1;
}
