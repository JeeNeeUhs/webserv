#include "Logger.hpp"

bool Logger::_debugMode = false;

Logger::Logger() {}

Logger::Logger(const Logger& other) {
	(void)other;
}

Logger& Logger::operator=(const Logger& other) {
	(void)other;
	return *this;
}

Logger::~Logger() {}

static void printCurrentTime() {
	std::time_t		now = std::time(0);
	struct std::tm*	tstruct = std::localtime(&now);
	char			buf[20];

	// YYYY-DD-MM HH:MM:SS
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %X", tstruct);
	std::cout << MAGENTA <<  "[" << std::string(buf) << "] " << RESET << std::flush;
}

void Logger::info(const std::string& msg) {
	printCurrentTime();
	std::cout << GREEN << "INFO \t" << RESET << msg << std::endl;
}

void Logger::warning(const std::string& msg) {
	printCurrentTime();
	std::cout << YELLOW << "WARN \t" << RESET << msg << std::endl;
}

void Logger::error(const std::string& msg) {
	printCurrentTime();
	std::cout << RED << "ERROR\t" << RESET << msg << std::endl;
}

void Logger::debug(const std::string& msg) {
	if (!_debugMode)
		return;

	printCurrentTime();
	std::cout << CYAN << "DEBUG\t" << RESET << msg << std::endl;
}

void Logger::setDebugMode(bool mode) {
	_debugMode = mode;
}
