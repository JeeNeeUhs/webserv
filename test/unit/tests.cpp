#include <iostream>

size_t run_http_tests(void);

int main() {
	std::cout << "webserv test runner started." << std::endl << std::endl;

	size_t total_failed = 0;

	total_failed += run_http_tests();

	if (total_failed == 0) {
		std::cout << "all tests passed." << std::endl;
		return 0;
	} else {
		std::cerr << "total " << total_failed << " tests failed." << std::endl;
		return 1;
	}
}
