#include "Config.hpp"

Config::Config() {}

Config::Config(const Config& other) {
	*this = other;
}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		this->servers = other.servers;
	}
	return *this;
}

Config::~Config() {}
