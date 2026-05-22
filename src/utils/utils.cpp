#include <sstream>
#include <exception>
#include <climits>

namespace utils {
	int parseInt(const std::string& str) {
		if (str.empty())
			throw std::runtime_error("expected a number, but got an empty value");

		bool negative = false;
		size_t i = 0;

		if (str[0] == '+' || str[0] == '-') {
			negative = (str[0] == '-');
			i = 1;
			if (str.size() == 1)
				throw std::runtime_error("invalid number: " + str);
		}

		long result = 0;
		for (; i < str.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
				throw std::runtime_error("invalid number: " + str);

			result = result * 10 + (str[i] - '0');
			if (result > INT_MAX)
				throw std::runtime_error("number out of range: " + str);
		}

		return negative ? -static_cast<int>(result) : static_cast<int>(result);
	}

	std::string itos(const int& n) {
		std::stringstream ss;
		
		ss << n;
		return ss.str();
	}
}
