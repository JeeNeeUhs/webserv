#ifndef SESSIONHANDLER_HPP
# define SESSIONHANDLER_HPP

# include <map>
# include <string>
# include <ctime>

struct Session {
	std::string id;
	std::string clientIP;
	// std::time_t lastActivity;
};

class SessionHandler {
	private:
		std::map<std::string, Session> sessions;
		std::string generateSessionID();
	public:
		SessionHandler();
		SessionHandler(const SessionHandler& other);
		SessionHandler& operator=(const SessionHandler& other);
		~SessionHandler();

		std::string getOrCreateSession(const std::string& clientIP);
		// void checkSessionTimeouts();


};



#endif	
