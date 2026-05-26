#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>

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
}

#endif
