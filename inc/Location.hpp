#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>
# include <map>

class Server;

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
		std::map<int, std::string> error_pages;
		std::vector<std::string> methods;

		Server* parent;

	public:
		Location();
		Location(const Location& other);
		Location& operator=(const Location& other);
		~Location();

		const std::string& getPath() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		Server* getParent() const;

		void fill(Server* parent); // test purposes
		void fill2(Server* parent); // test purposes
};

#endif
