#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>
# include <vector>

namespace utils {
	template <typename T>
	T parseNum(const std::string& s) {
		std::istringstream iss(s);
		T value;

		iss >> value;

		if (iss.fail() || !iss.eof())
			throw std::runtime_error("invalid numeric value: " + s);

		return value;
	}

	template <typename T>
	std::string toString(const T& n) {
		std::ostringstream oss;
		oss << n;
		return oss.str();
	}

	std::vector<std::string> split(const std::string& s, char delim);
	std::string trim(const std::string& s);
	std::string trimCharset(const std::string& s, const std::string& charSet);
	std::string toHex(int value);
	std::string toChunked(const std::string& body);
	unsigned int parseAddr(const std::string& host);
}

#endif
