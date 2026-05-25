#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>

namespace utils {
	size_t parseSizeT(const std::string& s);

	template <typename T>
	std::string toString(const T& n) {
		std::ostringstream oss;
		oss << n;
		return oss.str();
	}
}

#endif
