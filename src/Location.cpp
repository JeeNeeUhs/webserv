#include "Location.hpp"
#include "Server.hpp"
#include <algorithm>

Location::Location()
	: path(""),
	root(""),
	index(""),
	autoindex(false),
	redirect_code(0),
	redirect_path(""),
	upload_store(""),
	parent(NULL) {}

Location::Location(const Location& other)
	: path(other.path),
	root(other.root),
	index(other.index),
	autoindex(other.autoindex),
	redirect_code(other.redirect_code),
	redirect_path(other.redirect_path),
	upload_store(other.upload_store),
	cgi_extensions(other.cgi_extensions),
	error_pages(other.error_pages),
	methods(other.methods),
	parent(other.parent) {}

Location& Location::operator=(const Location& other) {
	if (this != &other) {
		path = other.path;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		redirect_code = other.redirect_code;
		redirect_path = other.redirect_path;
		upload_store = other.upload_store;
		cgi_extensions = other.cgi_extensions;
		error_pages = other.error_pages;
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
	upload_store = "/var/www/uploads";
	cgi_extensions.push_back(".py");
	methods.push_back("GET");
	this->parent = parent;
}

void Location::fill2(Server* parent) {
	path = "/cgi-index";
	root = "/Users/nothing/cowd/test";
	index = "index.py";
	autoindex = false;
	upload_store = "/var/www/uploads";
	cgi_extensions.push_back(".py");
	methods.push_back("GET");
	this->parent = parent;
}
