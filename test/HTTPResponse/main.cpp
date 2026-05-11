#include "HTTPResponse.hpp"
#include <iostream>

int main() {
	HTTPResponse response(200, "OK");
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", "48");
	response.addHeader("Connection", "close");
	response.setBody("<html><body><h1>Hello, World!</h1></body></html>");


	std::string responseString = response.toString();
	std::cout << responseString << std::endl;

	return 0;
}