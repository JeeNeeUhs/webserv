#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <map>

class HTTPRequest {
	private:
		std::string _method;
		std::string _path;
		std::string _protocol;
		std::string _version;

		std::map<std::string, std::string> _queries;
		std::map<std::string, std::string> _headers;

		std::string _body;

		void _parseQueryString(const std::string& queryStr);
		std::string _parseChunkedBody(const std::string& raw) const;

	public:
		HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		~HTTPRequest();

		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getProtocol() const;
		const std::string& getVersion() const;
		const std::map<std::string, std::string>& getQueries() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		std::string getHeader(const std::string& key) const;

		bool validate() const;
		bool parse(const std::string& rawRequest);
};

#endif
