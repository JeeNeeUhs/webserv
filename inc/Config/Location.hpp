#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

typedef struct s_error_page {
	int code;
	std::string path;
} t_error_page;

typedef enum e_parent_type {
	SERVER,
	LOCATION
} t_parent_type;

class Location {
	private:
		std::string path;
		std::string root;
		std::string index;
		bool autoindex;
		
		int redirect_code = -1;
		std::string redirect_path;
		std::string upload_store;

		std::vector<std::string> cgi_extensions;
		std::vector<t_error_page> error_pages;
		std::vector<std::string> methods;

		std::vector<Location> locations;
		t_parent_type parent_type;
		void* parent;

	public:
		Location();
		Location(const Location& other);
		Location& operator=(const Location& other);
		~Location();

};

#endif
