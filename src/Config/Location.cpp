#include "Location.hpp"

Location::Location() {}

Location::Location(const Location& other) {
	*this = other;
}

Location& Location::operator=(const Location& other) {
	if (this != &other) {
		path = other.path;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		cgi_path = other.cgi_path;
		redirect_code = other.redirect_code;
		redirect_path = other.redirect_path;
		upload_store = other.upload_store;
		cgi_extensions = other.cgi_extensions;
		error_pages = other.error_pages;
		methods = other.methods;
	}
	return *this;
}

Location::~Location() {}
