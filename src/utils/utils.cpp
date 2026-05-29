#include <sstream>
#include <exception>
#include "utils.hpp"

std::vector<std::string> utils::split(const std::string& s, char delim) {
	std::vector<std::string> tokens;
	std::stringstream ss(s);
	std::string token;

	while (std::getline(ss, token, delim)) {
		tokens.push_back(trim(token));
	}
	return tokens;
}

std::string utils::trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";

	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}
