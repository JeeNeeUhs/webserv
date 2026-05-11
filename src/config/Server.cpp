#include "Server.hpp"

// Tüm scalar üyeleri varsayılan değerleriyle başlatır
// sockFd -1: henüz bir socket açılmadığını gösterir
Server::Server() : sockFd(-1), host(""), port(0), client_max_body_size(0), root(""), index(""), autoindex(false) {}

Server::Server(const Server& other) : host(other.host), port(other.port),
	client_max_body_size(other.client_max_body_size), root(other.root),
	index(other.index), autoindex(other.autoindex), cgi_extensions(other.cgi_extensions),
	error_pages(other.error_pages), methods(other.methods), locations(other.locations) {}

Server& Server::operator=(const Server& other) {
	if (this != &other) {
		sockFd = other.sockFd;
		host = other.host;
		port = other.port;
		client_max_body_size = other.client_max_body_size;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		cgi_extensions = other.cgi_extensions;
		error_pages = other.error_pages;
		methods = other.methods;
		locations = other.locations;
	}
	return *this;
}

Server::~Server() {}

// --- Getters ---

int Server::getSockFd() const { return sockFd; }
std::string Server::getHost() const { return host; }
int Server::getPort() const { return port; }
size_t Server::getClientMaxBodySize() const { return client_max_body_size; }
std::string Server::getRoot() const { return root; }
std::string Server::getIndex() const { return index; }
bool Server::getAutoindex() const { return autoindex; }

// Vektörler referans olarak döndürülür: gereksiz kopya maliyetinden kaçınmak için
const std::vector<std::string>& Server::getCgiExtensions() const { return cgi_extensions; }
const std::vector<t_error_page>& Server::getErrorPages() const { return error_pages; }
const std::vector<std::string>& Server::getMethods() const { return methods; }
const std::vector<Location>& Server::getLocations() const { return locations; }

// --- Setters ---

void Server::setSockFd(int fd) { sockFd = fd; }
void Server::setHost(const std::string& h) { host = h; }
void Server::setPort(int p) { port = p; }
void Server::setClientMaxBodySize(size_t size) { client_max_body_size = size; }
void Server::setRoot(const std::string& r) { root = r; }
void Server::setIndex(const std::string& i) { index = i; }
void Server::setAutoindex(bool a) { autoindex = a; }

// Vektöre yeni eleman ekleyen yardımcı setter'lar
void Server::addCgiExtension(const std::string& ext) { cgi_extensions.push_back(ext); }
void Server::addErrorPage(const t_error_page& page) { error_pages.push_back(page); }
void Server::addMethod(const std::string& method) { methods.push_back(method); }
void Server::addLocation(const Location& location) { locations.push_back(location); }
