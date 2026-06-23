#include "SessionHandler.hpp"
#include "Logger.hpp"
#include "webserv.hpp"
#include "HTTPParser.hpp"
#include <vector>
#include "utils.hpp"


SessionHandler::SessionHandler() {
	std::srand(static_cast<unsigned int>(std::time(NULL)));
}

SessionHandler::SessionHandler(const SessionHandler& other) {
	sessions = other.sessions;
}

SessionHandler& SessionHandler::operator=(const SessionHandler& other) {
	if (this != &other) {
		sessions = other.sessions;
	}
	return *this;
}

SessionHandler::~SessionHandler() {}

std::string SessionHandler::generateSessionID() {
	std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int n = alphanum.size();
	std::string sessionID;
	sessionID.reserve(16);
	for (size_t i = 0; i < 16; ++i) {
		sessionID += alphanum[std::rand() % n];
	}
	return sessionID;
}

void SessionHandler::createSession(Connection& c) {
	std::string sessionID = generateSessionID();
	Session newSession;
	newSession.id = sessionID;
	newSession.creationTime = std::time(NULL);
	newSession.numOfReq = 1;
	sessions[sessionID] = newSession;
	Logger::debug("created new session with ID: " + sessionID);
	// Add Set-Cookie header to the response
	c.res.addHeader("Set-Cookie", "sid=" + sessionID + "; Path=/; Max-Age=" + utils::toString(SESSION_TIMEOUT));
}

void SessionHandler::debugSessionData(std::string sid) {
	std::map<std::string, Session>::iterator it = sessions.find(sid);
	if (it != sessions.end()) {
		const Session& session = it->second;
		Logger::debug("session ID: " + session.id);
		Logger::debug("creation time: " + utils::toString(session.creationTime));
		Logger::debug("number of requests: " + utils::toString(session.numOfReq));
	} else {
		Logger::debug("session ID not found: " + sid);
	}
}

void SessionHandler::getOrCreateSession(Connection& c) {
	std::vector<std::string> cookies = utils::split(HTTPParser::peekHeader(c.readBuff, "Cookie"), ';');
	std::vector<std::string>::iterator it;
	for (it = cookies.begin(); it != cookies.end(); ++it) {
		std::string cookie = utils::trim(*it);
		if (cookie.find("sid=") == 0) {
			std::string sid = cookie.substr(4);
			std::map<std::string, Session>::iterator sit = sessions.find(sid);
			if (sit != sessions.end()) {
				sit->second.numOfReq++;
				debugSessionData(sit->first);
				return;
			}
			break;
		}
	}
	createSession(c);
	return;
}

void SessionHandler::cleanExpiredSessions() {
	std::time_t now = std::time(NULL);
	std::map<std::string, Session>::iterator it = sessions.begin();
	while (it != sessions.end()) {
		if (now - it->second.creationTime > SESSION_TIMEOUT)
			sessions.erase(it++);
		else
			++it;
	}
}
