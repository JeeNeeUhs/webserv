#include "Location.hpp"
#include "Server.hpp"
#include <algorithm>

Location::Location()
	: path(""),
	root(""),
	index(""),
	autoindex(false),
	uploadStore(""),
	parent(NULL) {}

Location::Location(const Location& other) {
	operator=(other);
}

Location& Location::operator=(const Location& other) {
	if (this != &other) {
		path = other.path;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		uploadStore = other.uploadStore;
		cgiExtensions = other.cgiExtensions;
		errorPages = other.errorPages;
		methods = other.methods;
		parent = other.parent;
	}

	return *this;
}

Location::~Location() {}

const std::string& Location::getPath() const {
	return path;
}

const std::string& Location::getRoot() const {
	return root;
}

const std::string& Location::getIndex() const {
	return index;
}

Server* Location::getParent() const {
	return parent;
}

void Location::fill(Server* parent) {
	path = "/test";
	root = "/Users/nothing/cowd/test";
	index = "script.py";
	autoindex = false;
	uploadStore = "/var/www/uploads";
	cgiExtensions.push_back(".py");
	methods.push_back("GET");
	this->parent = parent;
}

void Location::fill2(Server* parent) {
	path = "/cgi-index";
	root = "/Users/nothing/cowd/test";
	index = "index.py";
	autoindex = false;
	uploadStore = "/var/www/uploads";
	cgiExtensions.push_back(".py");
	methods.push_back("GET");
	this->parent = parent;
}
