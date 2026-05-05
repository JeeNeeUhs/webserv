#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

typedef struct s_error_page {
	int code;
	std::string path;
} t_error_page;

class Location {
	private:
		std::string path;
		std::string root;
		std::string index;
		bool autoindex;
		
		int redirect_code;
		std::string redirect_path;
		std::string upload_store;

		std::vector<std::string> cgi_extensions;
		std::vector<t_error_page> error_pages;
		std::vector<std::string> methods;

	public:
		Location();
		Location(const Location& other);
		Location& operator=(const Location& other);
		~Location();

		std::string getPath() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoindex() const;
		int getRedirectCode() const;
		std::string getRedirectPath() const;
		std::string getUploadStore() const;
		std::vector<std::string> getCgiExtensions() const;
		std::vector<t_error_page> getErrorPages() const;
		std::vector<std::string> getMethods() const;
		std::vector<Location>& getLocations();
		
		void fill(); // test purposes

};

#endif
