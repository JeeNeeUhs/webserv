#include "StaticHandler.hpp"
#include "HTTPRequest.hpp"
#include "HTTPParser.hpp"
#include "RequestHandler.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

static bool getFilename(const std::string& raw, std::string& filename) {
	std::map<std::string, std::string> headers;
	if (!HTTPParser::parseHeaders(raw, headers))
		return false;
	
	std::vector<std::string> cd = utils::split(headers["Content-Disposition"], ';');
	for (size_t i = 0; i < cd.size(); ++i) {
		size_t pos = cd[i].find("filename=");
		if (pos != std::string::npos) {
			filename = cd[i].substr(pos + 10);
			while (!filename.empty() && (filename.back() == '\r' || filename.back() == ' '|| filename.back() == '"'))
				filename.pop_back();
			return true;
		}
	}
	return false;
}

// bool StaticHandler::uploadToStore(const HTTPRequest& req, const std::string& storePath) {
// 	struct stat st;
// 	if (stat(storePath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
// 		return false;	

// 	std::vector<std::string> ct = req.getHeaderValues("Content-Type");
// 	if (ct.empty() || ct[0].find("multipart/form-data") == std::string::npos)
// 		return false;

// 	std::string boundary;
// 	for (size_t i = 0; i < ct.size(); ++i) {
// 		size_t pos = ct[i].find("boundary=");
// 		if (pos != std::string::npos) {
// 			boundary = "--" + ct[i].substr(pos + 9);
// 			while (!boundary.empty() && (boundary.back() == '\r' || boundary.back() == ' '))
// 				boundary.pop_back();
// 			break;
// 		}
// 	}
// 	if (boundary.empty())
// 		return false;

// 	const std::string& body = req.getBody();

// 	if (body.find(boundary) == std::string::npos)
// 		return false;
// 	if (body.find(boundary + "--") == std::string::npos)
// 		return false;

// 	bool atLeast = false;
// 	size_t pos = 0;

// 	while (pos != std::string::npos) {
// 		pos = body.find(boundary, pos);
// 		if (pos == std::string::npos)
// 			break;
// 		pos += boundary.size();
		

// 		if (body.compare(pos, 2, "--") == 0)
// 			break;
		
// 		if (body.compare(pos, 2, "\r\n") != 0)
// 			break;
// 		Logger::debug("HERE");
// 		pos += 2;
// 		size_t headerEnd = body.find("\r\n\r\n", pos);
// 		if (headerEnd == std::string::npos)
// 			break;

// 		Logger::debug("HERE");

// 		std::string filename;
// 		if (!getFilename(body.substr(pos, headerEnd - pos), filename))
// 			return false; // sunu kontrol etmek lazim anamin paketi hatali oldugu icinmi error aticaz 
// 			//yoksa biz mi internal server error yiyioruz

// 		Logger::debug("HERE");

// 		pos = headerEnd + 4;

// 		//sikilmemek icin kontrol
// 		if (filename.find_last_of("/\\") != std::string::npos)
// 			filename = filename.substr(filename.find_last_of("/\\") + 1); // istersen patlat
// 		if (filename.empty() || filename == "." || filename == "..")
// 			return false;

// 		//duplicate duplicate duplicate duplicate orginal orginal orginal orginal
// 		std::string fullPath = storePath + "/" + filename;
// 		struct stat fst;
// 		if (stat(fullPath.c_str(), &fst) == 0)
// 			return false; // duplicate internal server error donmek lazim yada bir cozum bulmak emin degilim
		
// 		size_t nextBoundary = body.find(boundary, pos);
// 		if (nextBoundary == std::string::npos)
// 			return false;

// 		Logger::debug("HERE");
		
// 		size_t contentEnd = nextBoundary;
// 		if (contentEnd >= 2 && body.compare(contentEnd - 2, 2, "\r\n") == 0)
// 			contentEnd -= 2;

// 		std::string content = body.substr(pos, contentEnd - pos);
// 		std::ofstream outFile(fullPath, std::ios::binary);
// 		if (!outFile.is_open())
// 			return false;
// 		outFile.write(content.c_str(), content.size());
// 		if (!outFile.good()) {
// 			outFile.close();
// 			return false;
// 		}
// 		outFile.close();

// 		atLeast = true;
// 		pos = nextBoundary;
// 	}
// 	return atLeast;
// }

// HTTPResponse RequestHandler::uploadToStore(Connection& c) {


// }

enum status{
	WAIT,
	DONE,
	FAIL,
	NEXT
};

static int findBoundary(Connection& c) {
	size_t pos = c.readBuff.find(c.boundary);
	if (pos == std::string::npos)
		return c.uploadEof ? FAIL : WAIT;

	size_t after = pos + c.boundary.size();
	if (c.readBuff.size() < after + 2)
		return c.uploadEof ? FAIL : WAIT;

	if (c.readBuff.compare(after, 2, "--") == 0)
		return c.uploadedFiles.empty() ? FAIL : DONE;
	if (c.readBuff.compare(after, 2, "\r\n") != 0)
		return FAIL;

	c.readBuff.erase(0, after + 2);
	c.nmft = UPLOAD_PARSE_HEADER;
	return NEXT;
}

static int parseHeader(Connection& c) {
	size_t headerEnd = c.readBuff.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return c.uploadEof ? FAIL : WAIT;

	std::string partHeaders = c.readBuff.substr(0, headerEnd);
	c.readBuff.erase(0, headerEnd + 4);

	std::string filename;
	if (!getFilename(partHeaders, filename))
		return FAIL;

	size_t slash = filename.find_last_of("/\\");
	if (slash != std::string::npos)
		filename = filename.substr(slash + 1);
	if (filename.empty() || filename == "." || filename == "..")
		return FAIL;

	std::string fullPath = c.loc->uploadStore + "/" + filename;
	c.bodyFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (c.bodyFd == -1)
		return FAIL;

	c.uploadedFilename = filename;
	c.uploadedFiles.push_back(fullPath);
	c.nmft = UPLOAD_WRITE_BODY;
	return NEXT;
}

static int writeBody(Connection& c) {
	size_t pos = c.readBuff.find(c.boundary);

	if (pos == std::string::npos) {
		if (c.uploadEof)
			return FAIL;
		size_t keep = c.boundary.size() - 1;
		if (c.readBuff.size() <= keep)
			return WAIT;
		size_t writeLen = c.readBuff.size() - keep;
		ssize_t w = write(c.bodyFd, c.readBuff.c_str(), writeLen);
		if (w < 0 || (size_t)w != writeLen)
			return FAIL;
		c.readBuff.erase(0, writeLen);
		return WAIT;
	}

	size_t contentEnd = pos;
	if (contentEnd >= 2 && c.readBuff.compare(contentEnd - 2, 2, "\r\n") == 0)
		contentEnd -= 2;
	ssize_t w = write(c.bodyFd, c.readBuff.c_str(), contentEnd);
	if (w < 0 || (size_t)w != contentEnd)
		return FAIL;

	close(c.bodyFd);
	c.bodyFd = -1;
	c.readBuff.erase(0, pos);
	c.nmft = UPLOAD_FIND_BOUNDARY;
	return NEXT;
}

HTTPResponse RequestHandler::uploadToStore(Connection& c) {
	while (true) {
		int s;
		if (c.nmft == UPLOAD_FIND_BOUNDARY)
			s = findBoundary(c);
		else if (c.nmft == UPLOAD_PARSE_HEADER)
			s = parseHeader(c);
		else
			s = writeBody(c);

		if (s == NEXT)
			continue;
		if (s == WAIT)
			return HTTPResponse();
		if (s == DONE)
			return buildErrorResponse(*c.loc, 201);

		// FAIL -> inline cleanup
		if (c.bodyFd != -1) {
			close(c.bodyFd);
			c.bodyFd = -1;
		}
		for (size_t i = 0; i < c.uploadedFiles.size(); ++i)
			remove(c.uploadedFiles[i].c_str());
		return buildErrorResponse(*c.loc, 500);
	}
}
