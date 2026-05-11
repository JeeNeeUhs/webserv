#include "Server.hpp"
#include "HTTPRequest.hpp"
#include "cgi.hpp"
#include <iostream>


int main() {
	Server server;
	server.fill();
	HTTPRequest request;
	request.fill1();

	std::cout << "cgi output:" << cgiRun(server.getLocations()[0], request) << std::endl;

	return 0;
}