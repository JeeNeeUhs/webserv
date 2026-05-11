#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

// HTTP hata sayfası: hata kodu ve dosya yolu
typedef struct s_error_page {
	int code;
	std::string path;
} t_error_page;

// Location'ın parent'ının türünü belirtir (Server mi, başka bir Location mı)
typedef enum e_parent_type {
	SERVER,
	LOCATION
} t_parent_type;

class Location {
	private:
		std::string path;          // Eşleştirilecek URL yolu (örn: "/upload")
		std::string root;          // Bu location için dosya sistemi kök dizini
		std::string index;         // Varsayılan index dosyası
		bool autoindex;            // Dizin listeleme açık/kapalı

		int redirect_code;         // HTTP yönlendirme kodu (-1 ise yönlendirme yok)
		std::string redirect_path; // Yönlendirme hedef yolu

		std::string upload_store;  // Yüklenen dosyaların saklanacağı dizin

		std::vector<std::string> cgi_extensions; // İzin verilen CGI uzantıları (örn: ".py")
		std::vector<t_error_page> error_pages;   // Bu location'a özel hata sayfaları
		std::vector<std::string> methods;         // İzin verilen HTTP metodları

		std::vector<Location> locations; // İç içe location blokları
		t_parent_type parent_type;       // Parent'ın türü (SERVER veya LOCATION)
		void* parent;                    // Parent nesnesine ham pointer

	public:
		Location();
		Location(const Location& other);
		Location& operator=(const Location& other);
		~Location();

		// --- Getters ---
		std::string getPath() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoindex() const;
		int getRedirectCode() const;
		std::string getRedirectPath() const;
		std::string getUploadStore() const;
		const std::vector<std::string>& getCgiExtensions() const;
		const std::vector<t_error_page>& getErrorPages() const;
		const std::vector<std::string>& getMethods() const;
		const std::vector<Location>& getLocations() const;
		t_parent_type getParentType() const;
		void* getParent() const;

		// --- Setters ---
		void setPath(const std::string& path);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void setAutoindex(bool autoindex);
		void setRedirect(int code, const std::string& path); // Yönlendirme kodu ve hedefini birlikte ayarlar
		void setUploadStore(const std::string& store);
		void addCgiExtension(const std::string& ext);
		void addErrorPage(const t_error_page& page);
		void addMethod(const std::string& method);
		void addLocation(const Location& location);
		void setParent(t_parent_type type, void* ptr);
};

#endif
