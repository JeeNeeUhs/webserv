#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

// ─── Canonical form ───────────────────────────────────────────────────────────

Config::Config() {}

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file: " + filename);
	std::ostringstream ss;
	ss << file.rdbuf();
	std::vector<t_token> tokens = tokenize(ss.str());
	parseConfig(tokens);
	validate();
}

Config::Config(const Config& other) { *this = other; }

Config& Config::operator=(const Config& other) {
	if (this != &other)
		servers = other.servers;
	return *this;
}

Config::~Config() {}

const std::vector<Server>& Config::getServers() const { return servers; }

// ─── Tokenizer (Adım 2) ───────────────────────────────────────────────────────

std::vector<t_token> Config::tokenize(const std::string& content) {
	std::vector<t_token> tokens;
	size_t i = 0;
	const size_t len = content.size();

	while (i < len) {
		if (std::isspace(content[i])) { i++; continue; }

		if (content[i] == '#') {
			while (i < len && content[i] != '\n') i++;
			continue;
		}

		t_token tok;
		if (content[i] == '{') {
			tok.type = TOKEN_OPEN_BRACE; tok.value = "{"; i++;
		} else if (content[i] == '}') {
			tok.type = TOKEN_CLOSE_BRACE; tok.value = "}"; i++;
		} else if (content[i] == ';') {
			tok.type = TOKEN_SEMICOLON; tok.value = ";"; i++;
		} else {
			size_t start = i;
			while (i < len && !std::isspace(content[i]) &&
				   content[i] != '{' && content[i] != '}' &&
				   content[i] != ';' && content[i] != '#')
				i++;
			tok.type = TOKEN_WORD;
			tok.value = content.substr(start, i - start);
		}
		tokens.push_back(tok);
	}

	t_token eof; eof.type = TOKEN_EOF; eof.value = "";
	tokens.push_back(eof);
	return tokens;
}

// ─── Parser helpers ───────────────────────────────────────────────────────────

static const t_token& expect(const std::vector<t_token>& tokens, size_t& i, t_token_type type) {
	if (i >= tokens.size() || tokens[i].type != type) {
		std::string got = (i < tokens.size()) ? ("'" + tokens[i].value + "'") : "EOF";
		throw std::runtime_error("Unexpected token " + got);
	}
	return tokens[i++];
}

static const std::string& nextWord(const std::vector<t_token>& tokens, size_t& i) {
	return expect(tokens, i, TOKEN_WORD).value;
}

static void expectSemicolon(const std::vector<t_token>& tokens, size_t& i) {
	expect(tokens, i, TOKEN_SEMICOLON);
}

// ─── parseConfig (Adım 3) ────────────────────────────────────────────────────

void Config::parseConfig(const std::vector<t_token>& tokens) {
	size_t i = 0;
	while (tokens[i].type != TOKEN_EOF) {
		if (tokens[i].type == TOKEN_WORD && tokens[i].value == "server") {
			i++;
			servers.push_back(parseServerBlock(tokens, i));
		} else {
			throw std::runtime_error("Expected 'server' block, got: '" + tokens[i].value + "'");
		}
	}
}

Server Config::parseServerBlock(const std::vector<t_token>& tokens, size_t& i) {
	Server server;
	expect(tokens, i, TOKEN_OPEN_BRACE);

	while (tokens[i].type != TOKEN_CLOSE_BRACE && tokens[i].type != TOKEN_EOF) {
		if (tokens[i].type != TOKEN_WORD)
			throw std::runtime_error("Expected directive name in server block");
		std::string name = tokens[i++].value;

		if (name == "location") {
			server.addLocation(parseLocationBlock(tokens, i));
		} else {
			parseServerDirective(name, tokens, i, server);
		}
	}
	expect(tokens, i, TOKEN_CLOSE_BRACE);
	return server;
}

Location Config::parseLocationBlock(const std::vector<t_token>& tokens, size_t& i) {
	Location loc;
	loc.setPath(nextWord(tokens, i));
	expect(tokens, i, TOKEN_OPEN_BRACE);

	while (tokens[i].type != TOKEN_CLOSE_BRACE && tokens[i].type != TOKEN_EOF) {
		if (tokens[i].type != TOKEN_WORD)
			throw std::runtime_error("Expected directive name in location block");
		std::string name = tokens[i++].value;

		if (name == "location") {
			loc.addLocation(parseLocationBlock(tokens, i));
		} else {
			parseLocationDirective(name, tokens, i, loc);
		}
	}
	expect(tokens, i, TOKEN_CLOSE_BRACE);
	return loc;
}

// ─── Direktif parser'ları (Adım 4) ───────────────────────────────────────────

void Config::parseServerDirective(const std::string& name,
	const std::vector<t_token>& tokens, size_t& i, Server& server)
{
	if (name == "listen") {
		const std::string& val = nextWord(tokens, i);
		size_t colon = val.find(':');
		if (colon != std::string::npos) {
			server.setHost(val.substr(0, colon));
			server.setPort(std::atoi(val.substr(colon + 1).c_str()));
		} else {
			server.setPort(std::atoi(val.c_str()));
		}
		expectSemicolon(tokens, i);
	} else if (name == "host") {
		server.setHost(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "root") {
		server.setRoot(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "index") {
		server.setIndex(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "autoindex") {
		const std::string& val = nextWord(tokens, i);
		if (val != "on" && val != "off")
			throw std::runtime_error("autoindex must be 'on' or 'off', got: '" + val + "'");
		server.setAutoindex(val == "on");
		expectSemicolon(tokens, i);
	} else if (name == "client_max_body_size") {
		server.setClientMaxBodySize(parseSize(nextWord(tokens, i)));
		expectSemicolon(tokens, i);
	} else if (name == "error_page") {
		t_error_page ep;
		ep.code = std::atoi(nextWord(tokens, i).c_str());
		ep.path = nextWord(tokens, i);
		server.addErrorPage(ep);
		expectSemicolon(tokens, i);
	} else if (name == "cgi_extension") {
		server.addCgiExtension(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "methods") {
		while (tokens[i].type == TOKEN_WORD)
			server.addMethod(tokens[i++].value);
		expectSemicolon(tokens, i);
	} else {
		throw std::runtime_error("Unknown server directive: '" + name + "'");
	}
}

void Config::parseLocationDirective(const std::string& name,
	const std::vector<t_token>& tokens, size_t& i, Location& loc)
{
	if (name == "root") {
		loc.setRoot(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "index") {
		loc.setIndex(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "autoindex") {
		const std::string& val = nextWord(tokens, i);
		if (val != "on" && val != "off")
			throw std::runtime_error("autoindex must be 'on' or 'off', got: '" + val + "'");
		loc.setAutoindex(val == "on");
		expectSemicolon(tokens, i);
	} else if (name == "methods") {
		while (tokens[i].type == TOKEN_WORD)
			loc.addMethod(tokens[i++].value);
		expectSemicolon(tokens, i);
	} else if (name == "redirect") {
		int code = std::atoi(nextWord(tokens, i).c_str());
		const std::string path = nextWord(tokens, i);
		loc.setRedirect(code, path);
		expectSemicolon(tokens, i);
	} else if (name == "upload_store") {
		loc.setUploadStore(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "cgi_extension") {
		loc.addCgiExtension(nextWord(tokens, i));
		expectSemicolon(tokens, i);
	} else if (name == "error_page") {
		t_error_page ep;
		ep.code = std::atoi(nextWord(tokens, i).c_str());
		ep.path = nextWord(tokens, i);
		loc.addErrorPage(ep);
		expectSemicolon(tokens, i);
	} else {
		throw std::runtime_error("Unknown location directive: '" + name + "'");
	}
}

// ─── Size parser — "1M" → 1048576, "512K" → 524288 ──────────────────────────

size_t Config::parseSize(const std::string& value) {
	if (value.empty())
		throw std::runtime_error("Invalid size: empty value");

	size_t multiplier = 1;
	std::string numPart = value;
	char suffix = value[value.size() - 1];

	if (suffix == 'G' || suffix == 'g') { multiplier = 1024UL * 1024 * 1024; numPart = value.substr(0, value.size() - 1); }
	else if (suffix == 'M' || suffix == 'm') { multiplier = 1024 * 1024; numPart = value.substr(0, value.size() - 1); }
	else if (suffix == 'K' || suffix == 'k') { multiplier = 1024; numPart = value.substr(0, value.size() - 1); }

	if (numPart.empty() || !std::isdigit((unsigned char)numPart[0]))
		throw std::runtime_error("Invalid size value: '" + value + "'");

	return static_cast<size_t>(std::atol(numPart.c_str())) * multiplier;
}

// ─── Validation (Adım 5) ─────────────────────────────────────────────────────

static bool isValidMethod(const std::string& m) {
	return m == "GET" || m == "POST" || m == "DELETE" || m == "HEAD";
}

static bool isValidRedirectCode(int code) {
	return code == 301 || code == 302 || code == 303 || code == 307 || code == 308;
}

void Config::validate() {
	if (servers.empty())
		throw std::runtime_error("Config has no server blocks");

	for (size_t s = 0; s < servers.size(); s++) {
		const Server& srv = servers[s];

		if (srv.getPort() == 0)
			throw std::runtime_error("Server block missing 'listen' directive");
		if (srv.getPort() < 1 || srv.getPort() > 65535) {
			std::ostringstream oss; oss << srv.getPort();
			throw std::runtime_error("Server port out of range: " + oss.str());
		}

		const std::vector<std::string>& smethods = srv.getMethods();
		for (size_t m = 0; m < smethods.size(); m++) {
			if (!isValidMethod(smethods[m]))
				throw std::runtime_error("Invalid HTTP method in server block: '" + smethods[m] + "'");
		}

		const std::vector<Location>& locs = srv.getLocations();
		for (size_t l = 0; l < locs.size(); l++) {
			const Location& loc = locs[l];

			if (loc.getPath().empty())
				throw std::runtime_error("Location block has empty path");

			const std::vector<std::string>& lmethods = loc.getMethods();
			for (size_t m = 0; m < lmethods.size(); m++) {
				if (!isValidMethod(lmethods[m]))
					throw std::runtime_error("Invalid HTTP method in location '" + loc.getPath() + "': '" + lmethods[m] + "'");
			}

			if (loc.getRedirectCode() != -1 && !isValidRedirectCode(loc.getRedirectCode())) {
				std::ostringstream oss;
				oss << loc.getRedirectCode();
				throw std::runtime_error("Invalid redirect code in location '" + loc.getPath() + "': " + oss.str());
			}
		}
	}
}
