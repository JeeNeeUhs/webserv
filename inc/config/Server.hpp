#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include "Location.hpp"

class Server {
	private:
		int sockFd; // Dinleme socket'ının file descriptor'ı

		std::string host;                // Bind edilecek IP adresi (örn: "0.0.0.0")
		int port;                        // Dinlenecek port numarası
		size_t client_max_body_size;     // İstemcinin gönderebileceği maksimum body boyutu (byte)
		std::string root;                // Varsayılan dosya sistemi kök dizini
		std::string index;               // Varsayılan index dosyası
		bool autoindex;                  // Dizin listeleme açık/kapalı

		std::vector<std::string> cgi_extensions; // İzin verilen CGI uzantıları (örn: ".py", ".go")
		std::vector<t_error_page> error_pages;   // Hata kodlarına karşılık gelen sayfa yolları
		std::vector<std::string> methods;         // İzin verilen HTTP metodları
		std::vector<Location> locations;          // Bu server'a ait location blokları

	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();

		// --- Getters ---
		int getSockFd() const;
		std::string getHost() const;
		int getPort() const;
		size_t getClientMaxBodySize() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoindex() const;
		const std::vector<std::string>& getCgiExtensions() const;
		const std::vector<t_error_page>& getErrorPages() const;
		const std::vector<std::string>& getMethods() const;
		const std::vector<Location>& getLocations() const;

		// --- Setters ---
		void setSockFd(int fd);
		void setHost(const std::string& host);
		void setPort(int port);
		void setClientMaxBodySize(size_t size);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void setAutoindex(bool autoindex);
		void addCgiExtension(const std::string& ext);
		void addErrorPage(const t_error_page& page);
		void addMethod(const std::string& method);
		void addLocation(const Location& location);
};

#endif
