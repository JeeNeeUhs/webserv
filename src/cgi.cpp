#include "cgi.hpp"

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <iostream>

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

std::map<std::string, std::string> buildEnv(const Server& data, const HTTPRequest& request, const std::string& filePath) {
	std::map<std::string, std::string> env;

	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"] = "webserv/1.0";

	return env;
}

char** mapToEnvp(const std::map<std::string, std::string>& env) {
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

std::string executeCgi(const std::string& filePath, const std::map<std::string, std::string>& env) {
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		throw std::runtime_error("Failed to create pipe");
	}
	pid_t pid = fork();
	if (pid == -1) {
		throw std::runtime_error("Failed to fork process");
	} else if (pid == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		char* argv[2];
		argv[0] = const_cast<char*>(filePath.c_str());
		argv[1] = NULL;

		char** envp = mapToEnvp(env);

		execve(argv[0], argv, envp);
		exit(1);
	} else {
		close(pipefd[1]);

		std::string output;
		char buffer[4096];
		ssize_t bytesRead;

		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
			output.append(buffer, bytesRead);
		}

		close(pipefd[0]);

		int status;
		waitpid(pid, &status, 0);

		return output;
	}
}

std::string cgiRun(const Location& data, HTTPRequest& request) {
	std::string filepath;
	std::map<std::string, std::string> env;

	if (request.getPath() == data.getPath()) {
		filepath = data.getRoot() + "/" + data.getIndex();
	} else {
		filepath = data.getRoot() + request.getPath();
	}

	std::cout << filepath << std::endl;

	env = buildEnv(*data.getParent(), request, filepath);

	return executeCgi(filepath, env);
}
