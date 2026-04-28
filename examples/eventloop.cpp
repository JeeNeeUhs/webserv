#include <poll.h>
#include <vector>
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

#define POLL_TIMEOUT 3000
#define HEADER_TIMEOUT 60
#define BODY_TIMEOUT 180

#define MAX_HEADER_SIZE 8 * 1024 // config'e dahil edilebilir, opsiyonel
#define MAX_BODY_SIZE 10 * 1024 * 1024

typedef struct pollfd pollfd_t;

enum ClientState {
	READING_HEADERS,
	READING_BODY,
	TIMEOUT,
	DONE
};

struct Client {
	int			serverFd;
	
	time_t		connStart;
	time_t		lastActivity;
	
	ClientState	state;
	std::string readBuff;
	std::string writeBuff;
	size_t		headerLength;
	size_t		contentLength;
};

class Server {
	private:
		int			_fd;
		std::string	_host;
		int			_port;

	public:
		Server(std::string host, int port) : _fd(-1), _host(host), _port(port) {}
		~Server() {
			if (_fd >= 0)
				close(_fd);
		}

		int createSocket() {
			_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (_fd < 0)
				return -1;

			int opt = 1;
			if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0
				|| setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
				return -1;

			if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
				return -1;

			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(_host.c_str());
			addr.sin_port = htons(_port);

			if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
				return -1;

			if (listen(_fd, SOMAXCONN) < 0)
				return -1;

			return _fd;
		}
};

class Webserv {
	private:
		std::map<int, Server*> _servers;
		std::map<int, Client> _clients;
		std::vector<pollfd_t> _pollFds;

		void checkTimeouts() {
			time_t currTime = time(NULL);

			std::map<int, Client>::iterator it;
			for (it = _clients.begin(); it != _clients.end(); ++it) {
				Client &c = it->second;

				if (c.state == TIMEOUT)
					continue;

				bool headerTimeout = (c.state == READING_HEADERS && (currTime - c.connStart > HEADER_TIMEOUT));
				bool bodyTimeout = (c.state == READING_BODY && (currTime - c.lastActivity > BODY_TIMEOUT));
				bool sendTimeout = (c.state == DONE && (currTime - c.lastActivity > BODY_TIMEOUT));

				if (headerTimeout || bodyTimeout || sendTimeout)
 					c.state = TIMEOUT;
			}
		}

		void acceptClients(int serverFd) {
			while (true) {
				int clientFd = accept(serverFd, NULL, NULL);
				if (clientFd < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK)
						break;
					// TODO: buradaki fd limit dolu oldugunda poll surekli
					// calisip cpu kullanimini arttirabilir
					if (errno == EMFILE || errno == ENFILE) {
						std::cerr << "reached max fd limit" << std::endl;
						break;
					}
					continue;
				}

				if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
					close(clientFd);
					continue;
				}

				Client c;
				c.serverFd = serverFd;
				c.state = READING_HEADERS;
				c.connStart = time(NULL);
				c.lastActivity = c.connStart;
				c.headerLength = 0;
				c.contentLength = 0;

				_clients[clientFd] = c;

				struct pollfd newPfd;
				newPfd.fd = clientFd;
				newPfd.events = POLLIN;
				newPfd.revents = 0;

				_pollFds.push_back(newPfd);

				std::cout << "new client, fd: " << clientFd << std::endl;
			}
		}

		// donus degeri connection'in acik olup olmadigini ifade ediyor
		// drain edilecek buffer yoksa (-1, EAGAIN)fd nonblocking
		// setlendiginden dolayi bu fd icin fonksiyondan cikis yapmali.
		bool readFromClient(pollfd_t &pfd, Client &c) {
			char reqChunk[4096];

			// read loop
			while (true) {
				int bytes = recv(pfd.fd, reqChunk, sizeof(reqChunk) - 1, 0);
				
				if (bytes > 0) {
					size_t reqSize = c.readBuff.size() + bytes;

					if (c.state == READING_HEADERS && reqSize > MAX_HEADER_SIZE) {
						std::cout << "fd " << pfd.fd << " sent too long header" << std::endl;
						return false;
					}

					// TODO: 413 too large
					if (c.state == READING_BODY) {
						size_t reqBodySize = reqSize - c.headerLength;
						if (reqBodySize > c.contentLength || reqBodySize > MAX_BODY_SIZE) {
							std::cout << "fd " << pfd.fd << " sent too large body" << std::endl;
							return false;
						}
					}

					c.readBuff.append(reqChunk, bytes);
				}
				else if (bytes == 0) { // eof (FIN packet)
					std::cout << "fd " << pfd.fd << " shut down gracefully" << std::endl;
					return false;
				}
				else
					break;
			}

			if (c.state == READING_HEADERS) {
				size_t pos = c.readBuff.find("\r\n\r\n");

				if (pos != std::string::npos) {
					c.headerLength = pos + 4;
					c.state = READING_BODY;

					// TODO: header parsing
				}
			}

			if (c.state == READING_BODY) {
				size_t bodySize = c.readBuff.size() - c.headerLength;

				if (bodySize >= c.contentLength) {
					c.state = DONE;

					// FIXME: burayi hard code yaptik duzeltcen tabii
					c.writeBuff = "HTTP/1.0 200 OK\r\n"
						"Content-Type: text/plain\r\n"
						"Content-Length: 12\r\n"
						"\r\n"
						"Hello World";

					// artik bu fd'den okuma eventi beklemiyoruz,
					// poll ile yazmaya hazir hale getiriyoruz
					pfd.events = POLLOUT;
				}

				// TODO: prepare http response and fill write buffer
			}

			return true;
		}

		// bu noktada veri elimizde hazir. recv gibi drain etmemiz gereken
		// bir alan yok. bu sebeple ekstra bir while dongusu acmadan eventLoop
		// dongusu ile POLLOUT durumundaki socketimizi her poll
		bool sendToClient(int fd, Client &c) {
			if (c.writeBuff.empty()) 
				return false;

			int bytes = send(fd, c.writeBuff.c_str(), c.writeBuff.size(), 0);

			if (bytes > 0) {
				c.writeBuff.erase(0, bytes);
				
				if (c.writeBuff.empty()) {
					std::cout << "fd " << fd << " response sent gracefully" << std::endl;
					return false;
				}

				// veriler bitmedi, devam et
				return true;
			}
			
			// bytes <= 0 durumu
			std::cout << "fd " << fd << " send error" << std::endl;
			return false;
		}

		bool handleClients(pollfd_t &pfd, short revents) {
			std::map<int, Client>::iterator it = _clients.find(pfd.fd);
			if (it == _clients.end())
				return false;

			Client &c = it->second;

			// RST packet, hang up cases
			if (revents & (POLLERR | POLLHUP)) {
				std::cout << "fd " << pfd.fd << " hung up" << std::endl;
				return false;
			}

			if (revents & POLLIN) {
				if (!readFromClient(pfd, c))
					return false;
				c.lastActivity = time(NULL);
			}

			if (revents & POLLOUT) {
				if (!sendToClient(pfd.fd, c))
					return false;
				c.lastActivity = time(NULL);
			}

			return true;
		}

	public:
		bool initSockets(std::vector<Server*> &servers) {
			std::vector<Server*>::iterator it;

			for (it = servers.begin(); it != servers.end(); ++it) {
				int fd = (*it)->createSocket();
				if (fd < 0) {
					std::cerr << "failed while creating socket" << std::endl;
					return false;
				}

				pollfd_t pfd;
				pfd.fd = fd;
				pfd.events = POLLIN;
				pfd.revents = 0;

				_pollFds.push_back(pfd);
				_servers[fd] = *it;
			}

			return true;
		}

		void runEventLoop() {
			while (0x42) {
				checkTimeouts();

				int ready = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT);

				// bu kontrolu asagiya, ready > 0 icindeki loopta sagliyoruz artik.
				// sunucuda sadece 1 clientin bulunup sadece baglantisini acik biraktigi
				// durumda burasi calismayacakti
				// if (ready == 0)
				// 	continue;

				if (ready < 0) {
					if (errno == EINTR)
						continue;
					break;
				}

				for (size_t i = 0; i < _pollFds.size(); ++i) {
					struct pollfd &pfd = _pollFds[i];

					if (_servers.find(pfd.fd) != _servers.end()) {
						if (pfd.revents & POLLIN)
							acceptClients(pfd.fd);

						continue;
					}

					std::map<int, Client>::iterator it = _clients.find(pfd.fd);

					// pollfd listesi temizligini fonksiyonlarda ayri ayri
					// isleyerek karmasikligi arttirmak istemedigimden burada
					// -1 setliyorum ki artik gecersiz olan clientlarin fd'lerini
					// asagida daha rahat temizleyebileyim
					if (it != _clients.end() && it->second.state == TIMEOUT) {
						std::cout << "fd " << pfd.fd << " timeout" << std::endl;

						close(pfd.fd);
						_clients.erase(it);
						pfd.fd = -1;

						continue;
					}

					// if (ready == 0) kontrolune denk gelen kontrol
					if (pfd.revents == 0)
						continue;

					if (!handleClients(pfd, pfd.revents)) {
						close(pfd.fd);
						_clients.erase(pfd.fd);
						pfd.fd = -1;
					}

					// defensive programming type shi
					pfd.revents = 0;
				}

				// pollfd list cleanup part burasi
				for (size_t i = 0; i < _pollFds.size(); ) {
					if (_pollFds[i].fd == -1) {
						// .erase O(n) complexity'e sahip kaydirma yaptigindan
						// pop + swap islemleri O(1) complexity ile bu isi cozuyor
						_pollFds[i] = _pollFds.back();
						_pollFds.pop_back();
					} else {
						++i;
					}
				}
			}
		}
};

int main() {
	Webserv serv;

	Server s1("0.0.0.0", 8080);
	Server s2("127.0.0.1", 8081);

	std::vector<Server*> servers;
	servers.push_back(&s1);
	servers.push_back(&s2);

	if (!serv.initSockets(servers))
		return 1;
	serv.runEventLoop();
}
