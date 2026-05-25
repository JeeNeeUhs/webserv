#include <sstream>
#include <exception>

namespace utils {
	size_t parseSizeT(const std::string& s) {
		std::istringstream iss(s);
		size_t value;

		iss >> value;

		if (iss.fail() || !iss.eof())
			throw std::runtime_error("invalid numeric value: " + s);

		return value;
	}
}
