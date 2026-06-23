#ifndef SESSIONHANDLER_HPP
# define SESSIONHANDLER_HPP

# include <map>
# include <string>
# include <cstdlib>
# include <ctime>
# include "Connection.hpp"

struct Session {
	std::string id;
	std::time_t creationTime;
	int numOfReq;
};

class SessionHandler {
	private:
		std::map<std::string, Session> sessions;

		std::string generateSessionID();
		void createSession(Connection& c);
		void debugSessionData(std::string sid);
	public:
		SessionHandler();
		SessionHandler(const SessionHandler& other);

		SessionHandler& operator=(const SessionHandler& other);

		~SessionHandler();

		void getOrCreateSession(Connection& c);
		void cleanExpiredSessions();
};

#endif	
