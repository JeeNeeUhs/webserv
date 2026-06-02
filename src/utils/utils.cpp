#include <sstream>
#include <exception>
#include <netinet/in.h>
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

unsigned int utils::parseAddr(const std::string& host) {
	if (host == "0.0.0.0")
		return INADDR_ANY;

	std::istringstream iss(host);
	unsigned int hostIp = 0;
	unsigned int octet;
	char dot;

	for (int i = 0; i < 4; ++i) {
		if (!(iss >> octet) || octet > 255)
			throw std::runtime_error("invalid host address: " + host);

		hostIp = (hostIp << 8) | octet;
		if (i < 3 && (!(iss >> dot) || dot != '.'))
			throw std::runtime_error("invalid host address: " + host);
	}

	std::string leftover;
	if (iss >> leftover)
		throw std::runtime_error("invalid host address: " + host);

	return htonl(hostIp);
}
