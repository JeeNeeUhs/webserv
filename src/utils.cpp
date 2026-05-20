#include <sstream>
#include <exception>

int parseInt(const std::string& str) {
	std::istringstream ss(str);
	int num;

	if (!(ss >> num))
		throw std::runtime_error("error occured while parsing int value");

	return num;
}
