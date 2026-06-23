#include "webserv.hpp"
#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPParser.hpp"
#include "RequestHandler.hpp"
#include "utils.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "ServerManager.hpp"
#include "HTTPResponse.hpp"

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

extern char **environ;

// template<typename T>
// int checkCgiExtentions(const T& data, const std::string& path) {
// 	std::vector<std::string> cgi_extensions = data.getCgiExtensions();
// 	for (size_t i = 0; i < cgi_extensions.size(); ++i) {
// 		if (path.size() >= cgi_extensions[i].size() &&
// 			path.compare(path.size() - cgi_extensions[i].size(), cgi_extensions[i].size(), cgi_extensions[i]) == 0) {
// 			return i;
// 		}
// 	}
// 	return -1;
// }

static std::map<std::string, std::string> buildEnv(const ServerConfig& data, const LocationConfig& location, const HTTPRequest& req) {
	std::map<std::string, std::string> env;

	for (char** envEntry = environ; *envEntry != NULL; ++envEntry) {
		std::string entry(*envEntry);
		size_t pos = entry.find('=');

		if (pos != std::string::npos) {
			std::string key = entry.substr(0, pos);
			std::string value = entry.substr(pos + 1);
			env[key] = value;
		}
	}

	env["SERVER_PROTOCOL"] = CGI::SERVER_PROTOCOL;
	env["SERVER_SOFTWARE"] = CGI::SERVER_SOFTWARE;
	env["GATEWAY_INTERFACE"] = CGI::GATEWAY_INTERFACE;

	std::string querys;
	std::map<std::string, std::string>::const_iterator it;
	for (it = req.getQueries().begin(); it != req.getQueries().end(); ++it) {
		querys += it->first + "=" + it->second;
		std::map<std::string, std::string>::const_iterator next = it;
		++next;

		if (next != req.getQueries().end())
			querys += "&";
	}

	std::string scriptName;
	if (req.getPath() == location.path) {
		scriptName = location.root + "/" + location.index;
	} else {
		scriptName = location.root + req.getPath();
	}

	env["REQUEST_METHOD"] = req.getMethod();
	env["QUERY_STRING"] = querys;
	env["SCRIPT_NAME"] = scriptName;
	env["SERVER_PORT"] = utils::toString(data.listens[0].second);
	env["REMOTE_ADDR"] = utils::toString(data.listens[0].first);
	env["REMOTE_HOST"] = utils::toString(data.listens[0].first);
	env["SERVER_NAME"] = utils::toString(data.listens[0].first);
	env["CONTENT_TYPE"] = req.getHeader("Content-Type");
	env["CONTENT_LENGTH"] = req.getHeader("Content-Length").c_str();

	std::map<std::string, std::string>::const_iterator headerIt;
	for (headerIt = req.getHeaders().begin(); headerIt != req.getHeaders().end(); ++headerIt) {
		if (headerIt->first == "Content-Type" || headerIt->first == "Content-Length")
			continue;
		std::string headerName = "HTTP_" + headerIt->first;
		for (size_t i = 0; i < headerName.size(); ++i) {
			if (headerName[i] == '-')
				headerName[i] = '_';
			else
				headerName[i] = std::toupper(headerName[i]);
		}
		env[headerName] = headerIt->second;
	}

	return env;


	// env.push_back("SERVER_PROTOCOL=" + CGI::SERVER_PROTOCOL);
	// env.push_back("SERVER_SOFTWARE=" + CGI::SERVER_SOFTWARE);
	// env.push_back("GATEWAY_INTERFACE=" + CGI::GATEWAY_INTERFACE);

	// std::string methods;
	// for (size_t i = 0; i < location.methods.size(); ++i) {
	// 	methods += location.methods[i];
	// 	if (i != location.methods.size() - 1)
	// 		methods += ",";
	// }

	// std::string querys;
	// std::map<std::string, std::string>::const_iterator it;
	// for (it = req.getQueries().begin(); it != req.getQueries().end(); ++it) {
	// 	querys += it->first + "=" + it->second;
	// 	if (std::next(it) != req.getQueries().end())
	// 		querys += "&";
	// }

	// std::string scriptName;
	// if (req.getPath() == location.path) {
	// 	scriptName = location.root + "/" + location.index;
	// } else {
	// 	scriptName = location.root + req.getPath();
	// }

	// env.push_back("REQUEST_METHOD=" + methods);
	// env.push_back("QUERY_STRING=" + querys);
	// env.push_back("SCRIPT_NAME=" + scriptName);
	// env.push_back("SERVER_PORT=" + utils::toString(data.listens[0].second));
	// env.push_back("REMOTE_ADDR=");//sanirim su anda alamiyoruz ama bir yolu bulunacak
	// env.push_back("CONTENT_TYPE=" + req.getHeader("Content-Type"));
	// env.push_back("CONTENT_LENGTH=" + utils::toString(req.getBody().size()));

	// std::map<std::string, std::string>::const_iterator headerIt;
	// for (headerIt = req.getHeaders().begin(); headerIt != req.getHeaders().end(); ++headerIt) {
	// 	if (headerIt->first == "Content-Type" || headerIt->first == "Content-Length")
	// 		continue;
	// 	std::string headerName = "HTTP_" + headerIt->first;
	// 	for (size_t i = 0; i < headerName.size(); ++i) {
	// 		if (headerName[i] == '-')
	// 			headerName[i] = '_';
	// 		else
	// 			headerName[i] = std::toupper(headerName[i]);
	// 	}
	// 	env.push_back(headerName + "=" + headerIt->second);
	// }

	// return env;
}

static char** mapToEnvp(const std::map<std::string, std::string>& env) {
	char** envp = new char*[env.size() + 1];
	size_t i = 0;

	std::map<std::string, std::string>::const_iterator it;
	for (it = env.begin(); it != env.end(); ++it) {
		std::string entry = it->first + "=" + it->second;
		envp[i] = new char[entry.length() + 1];
		std::strcpy(envp[i], entry.c_str());
		++i;
	}
	envp[i] = NULL;
	return envp;
}

// std::string executeCgi(const std::string& filePath, const std::map<std::string, std::string>& env) {
// 	int pipefd[2];
// 	if (pipe(pipefd) == -1) {
// 		throw std::runtime_error("Failed to create pipe");
// 	}
// 	pid_t pid = fork();
// 	if (pid == -1) {
// 		throw std::runtime_error("Failed to fork process");
// 	} else if (pid == 0) {
// 		close(pipefd[0]);
// 		dup2(pipefd[1], STDOUT_FILENO);
// 		close(pipefd[1]);

// 		char* argv[2];
// 		argv[0] = const_cast<char*>(filePath.c_str());
// 		argv[1] = NULL;

// 		char** envp = mapToEnvp(env);

// 		execve(argv[0], argv, envp);
// 		exit(1);
// 	} else {
// 		close(pipefd[1]);

// 		std::string output;
// 		char buffer[4096];
// 		ssize_t bytesRead;

// 		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
// 			output.append(buffer, bytesRead);
// 		}

// 		close(pipefd[0]);

// 		int status;
// 		waitpid(pid, &status, 0);

// 		return output;
// 	}
// }

// void cgiRun(const Location& data, const HTTPRequest& request, HTTPResponse& response) {
// 	std::string filepath;
// 	std::map<std::string, std::string> env;

// 	if (request.getPath() == data.getPath()) {
// 		filepath = data.getRoot() + "/" + data.getIndex();
// 	} else {
// 		filepath = data.getRoot() + request.getPath();
// 	}

// 	std::cout << filepath << std::endl;

// 	env = buildEnv(*data.getParent(), request, filepath);

// 	std::string cgiOutput;
// 	cgiOutput = executeCgi(filepath, env);
// 	//parse cgiOutput to set response body and headers
	
// }

// bool cgiRun(const ServerConfig& data, const LocationConfig& location, const HTTPRequest& request) {
// 	std::vector<std::string> env = buildEnv(data, location, request);
	
// }
static pid_t executeCgi(Connection& c, std::string& filePath) {
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) == -1)
		return -1;
	if (pipe(pipeOut) == -1) {
		close(pipeIn[0]);
		close(pipeIn[1]);
		return -1;
	}

	Logger::debug("attempting to execute CGI script at " + filePath);

	pid_t pid = fork();
	if (pid == -1) {
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		return -1;
	} else if (pid == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		// Logger::debug("child process started for CGI execution");

		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);

		if (chdir(c.loc->root.c_str()) == -1)
			exit(1);

		std::map<std::string, std::string> env = buildEnv(*c.config, *c.loc, c.req);
		
		char* argv[2];
		argv[0] = const_cast<char*>(filePath.c_str());
		argv[1] = NULL;

		// Logger::debug("executing CGI script: " + filePath);
		execve(filePath.c_str(), argv, mapToEnvp(env));
		Logger::error("Failed to execute CGI script: " + filePath);
		perror("debug :");
		exit(1);
	} else {
		close(pipeIn[0]);
		close(pipeOut[1]);

		if (fcntl(pipeOut[0], F_SETFL, O_NONBLOCK) == -1) {
			close(pipeIn[1]);
			close(pipeOut[0]);
			kill(pid, SIGKILL);
			// waitpid(pid, NULL, 0);
			return -1;
		}

		if (fcntl(pipeIn[1], F_SETFL, O_NONBLOCK) == -1) {
			close(pipeIn[1]);
			close(pipeOut[0]);
			kill(pid, SIGKILL);
			// waitpid(pid, NULL, 0);
			return -1;
		}
		c.cgiPid = pid;
		c.cgiReadFd = pipeOut[0];
		c.cgiWriteFd = pipeIn[1];
	}
	return pid;
}

void RequestHandler::cgiDoneWriting(Connection& c, pollfd_t& pfd) {
	if (c.cgiWriteFd != -1) {
		close(c.cgiWriteFd);
		c.cgiWriteFd = -1;
	}
	if (pfd.fd != -1) {
		pfd.fd = -1;
	}
}

void RequestHandler::cgiDoneReading(Connection& c, pollfd_t& pfd) {
	if (c.cgiReadFd != -1) {
		close(c.cgiReadFd);
		c.cgiReadFd = -1;
	}
	if (pfd.fd != -1) {
		pfd.fd = -1;
	}
	if (c.cgiPid != -1) {
		kill(c.cgiPid, SIGKILL);
		waitpid(c.cgiPid, NULL, 0);
		c.cgiPid = -1;
	}
}

void RequestHandler::cgiDone(Connection& c, pollfd_t& pfd) {
	//cgiden donen hersey write buffta tutuluyor bu fonksiyon cagrildiginda bu calistigi zamanda soylebirsey olucak
	//simdi cgi donen hersey parsesiz ve headerlerde orda donuyor headeri parse ve request e ekle 
	//body write buffta kalabilir

	std::size_t headerEnd = HTTPParser::findHeaderEnd(c.cgiWriteBuff);
	c.headerLength = headerEnd;
	std::map<std::string, std::string> cgiHeaders;
	HTTPParser::parseHeaders(c.cgiWriteBuff.substr(0, headerEnd), cgiHeaders);
	c.cgiWriteBuff.erase(0, headerEnd);
	if (!c.cgiWriteBuff.empty())
		c.res.setBody(utils::toChunked(c.cgiWriteBuff));
	c.cgiWriteBuff.clear();

	c.state = CONN_DONE;
	pfd.events = POLLOUT;

	if (cgiHeaders.find("Content-Type") == cgiHeaders.end() || cgiHeaders["Content-Type"].empty()){
		c.res = buildErrorResponse(*c.config, 500);
		c.writeBuff = c.res.serialize();
		return;
	}
	if (cgiHeaders.find("Status") == cgiHeaders.end() || cgiHeaders["Status"].empty()) {
		c.res.setStatusCode(200);
	} else {
		std::vector<std::string> statusParts = utils::split(cgiHeaders["Status"], ' ');
		if (statusParts.size() < 2) {
			c.res = buildErrorResponse(*c.config, 500);
			c.writeBuff = c.res.serialize();
			return;
		}
		c.res.setStatusCode(utils::parseNum<size_t>(statusParts[0]));
		std::string reasonPhrase;
		for (size_t i = 1; i < statusParts.size(); ++i) 
			reasonPhrase += statusParts[i] + (i != statusParts.size() - 1 ? " " : "");
		c.res.setReasonPhrase(reasonPhrase);
	}
	for (std::map<std::string, std::string>::const_iterator it = cgiHeaders.begin(); it != cgiHeaders.end(); ++it) {
		if (it->first == "Status")
			continue;
		c.res.addHeader(it->first, it->second);
		c.res.addHeader("Transfer-Encoding", "chunked");
		Logger::debug("added CGI header: " + it->first + ": " + it->second);
	}

	c.writeBuff = c.res.serialize();
	c.nmft = UPLOAD_WRITE_BODY;
	return;
}

// cgi sisteme ful entegre olarak calisir
// asagida gormus oldugunuz fonksiyonu sadece cgi baslatmak icin procesess i olusturmak icin kullaniyoruz
// diger butun takipler pollda
// pollda bunu kullana nbilmek icin bazi caprazlamalar yapmamzi gerekiyor
// client to readfromcleint to sendtoclient to cgi to readfromclient to sendtoclient
// yukarida yazdigim seyde cgiye yakin olanlar clinet fdleri icin degilde cgi fdsi icin calisir
// ama biz cgi icin ayri read fln kullanmaktansa ayni fonksiyonlari kullaniyoruz
HTTPResponse RequestHandler::createCgi(Connection& c) {
	c.lastActivity = std::time(NULL);
	c.req.parse(c.readBuff, c.headerLength);
	c.readBuff.erase(0, c.headerLength);
	c.loc = matchLocation(*c.config, c.req.getPath());

	std::string filePath;
	if (utils::trimCharset(c.req.getPath(), "/") == utils::trimCharset(c.loc->path, "/")) {
		if (c.req.getPath()[c.req.getPath().size() - 1] != '/')
			filePath = c.loc->root + c.req.getPath() + "/" + c.loc->index;
		else
			filePath = c.loc->root + c.req.getPath() + c.loc->index;
	} else {
		filePath = c.loc->root + c.req.getPath();
	}

	if (!(access(filePath.c_str(), X_OK) == -1)) {
		executeCgi(c, filePath);
	}
	if (c.cgiPid < 0) {
		Logger::error("Failed to execute CGI script for path " + c.req.getPath());
		c.res = buildErrorResponse(*c.config, 500);
		c.writeBuff = c.res.serialize();
		c.state = CONN_DONE;
		return c.res;
	}
	Logger::debug("started CGI process with PID " + utils::toString(c.cgiPid) + " for fd " + utils::toString(c.cgiReadFd) + " and write fd " + utils::toString(c.cgiWriteFd));
	return HTTPResponse();
}