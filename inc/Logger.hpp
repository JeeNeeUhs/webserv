#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

// ANSI color escape codes
#define RED		"\033[31m"
#define GREEN	"\033[32m"
#define BLUE	"\033[34m"
#define YELLOW	"\033[33m"
#define CYAN	"\033[36m"
#define MAGENTA	"\033[35m"
#define RESET	"\033[0m"

class Logger {
	private:
		static bool _debugMode;

		Logger();
		Logger(const Logger& other);
		Logger& operator=(const Logger& other);
		~Logger();

	public:
		static void info(const std::string& msg);
		static void warning(const std::string& msg);
		static void error(const std::string& msg);
		static void debug(const std::string& msg);

		static void setDebugMode(bool mode);
};

#endif