#include "Location.hpp"
#include "Server.hpp"

Location::Location() : redirect_code(-1) {}

Location::Location(const Location& other) {
	*this = other;
}

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
	}
	return *this;
}

Location::~Location() {}

std::string Location::getPath() const {
	return path;
}

std::string Location::getRoot() const {
	return root;
}

std::string Location::getIndex() const {
	return index;
}

bool Location::getAutoindex() const {
	return autoindex;
}

int Location::getRedirectCode() const {
	return redirect_code;
}

std::string Location::getRedirectPath() const {
	return redirect_path;
}

std::string Location::getUploadStore() const {
	return upload_store;
}

std::vector<std::string> Location::getCgiExtensions() const {
	return cgi_extensions;
}

std::vector<t_error_page> Location::getErrorPages() const {
	return error_pages;
}

std::vector<std::string> Location::getMethods() const {
	return methods;
}

Server *Location::getParent() const {
	return parent;
}

void Location::fill() {

}
