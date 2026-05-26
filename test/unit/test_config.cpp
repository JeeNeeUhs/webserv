#include "ConfigParser.hpp"

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

static bool test_config_listens(void) {
	std::string source = "server {\nlisten 8080;listen 127.0.0.1:65535;}";

	ConfigParser cp(source);
	const std::vector<ServerConfig>& servers = cp.parse();

	std::string listen = servers[0].listens[0].first;
	std::string listen2 = servers[0].listens[1].first;

	if (!assert_equal(listen, std::string("8080")))
		return false;
	if (!assert_equal(listen2, std::string("127.0.0.1:65535")))
		return false;
	return true;
}

size_t run_config_tests(void) {
	size_t failed = 0;
	
	std::cout << "running config tests..." << std::endl;

	if (!test_config_listens())
		failed++;

	return failed;
}
