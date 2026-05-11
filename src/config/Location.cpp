#include "Location.hpp"

// Tüm scalar üyeleri varsayılan değerleriyle başlatır
// redirect_code -1: aktif bir yönlendirme olmadığını gösterir
// parent NULL olarak başlar, parse sırasında setParent() ile atanır
Location::Location() : autoindex(false), redirect_code(-1), parent_type(SERVER), parent(NULL) {}

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
		locations = other.locations;
		parent_type = other.parent_type;
		parent = other.parent;
	}
	return *this;
}

Location::~Location() {}

// --- Getters ---

std::string Location::getPath() const { return path; }
std::string Location::getRoot() const { return root; }
std::string Location::getIndex() const { return index; }
bool Location::getAutoindex() const { return autoindex; }
int Location::getRedirectCode() const { return redirect_code; }
std::string Location::getRedirectPath() const { return redirect_path; }
std::string Location::getUploadStore() const { return upload_store; }

const std::vector<std::string>& Location::getCgiExtensions() const { return cgi_extensions; }
const std::vector<t_error_page>& Location::getErrorPages() const { return error_pages; }
const std::vector<std::string>& Location::getMethods() const { return methods; }
const std::vector<Location>& Location::getLocations() const { return locations; }

t_parent_type Location::getParentType() const { return parent_type; }
void* Location::getParent() const { return parent; }

// --- Setters ---

void Location::setPath(const std::string& p) { path = p; }
void Location::setRoot(const std::string& r) { root = r; }
void Location::setIndex(const std::string& i) { index = i; }
void Location::setAutoindex(bool a) { autoindex = a; }

// Yönlendirme kodu ve hedef yolunu birlikte ayarlar
void Location::setRedirect(int code, const std::string& p) {
	redirect_code = code;
	redirect_path = p;
}

void Location::setUploadStore(const std::string& store) { upload_store = store; }

// Vektöre yeni eleman ekleyen yardımcı setter'lar
void Location::addCgiExtension(const std::string& ext) { cgi_extensions.push_back(ext); }
void Location::addErrorPage(const t_error_page& page) { error_pages.push_back(page); }
void Location::addMethod(const std::string& method) { methods.push_back(method); }
void Location::addLocation(const Location& loc) { locations.push_back(loc); }

// Parent pointer ve türünü birlikte ayarlar
// type: SERVER ise parent bir Server*, LOCATION ise bir Location* olarak yorumlanır
void Location::setParent(t_parent_type type, void* ptr) {
	parent_type = type;
	parent = ptr;
}
