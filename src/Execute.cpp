#include "Execute.hpp"

Execute::Execute() {}

Execute::Execute(const Execute& other) {
	this->servers = other.servers;
}

Execute& Execute::operator=(const Execute& other) {
	if (this != &other) {
		this->servers = other.servers;
	}
	return *this;
}

Execute::~Execute() {}