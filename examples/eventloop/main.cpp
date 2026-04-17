#include <poll.h>
#include <vector>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

/*
struct pollfd {
	int		fd;
	short	events;
	short	revents;
};
*/

class Server {
	private:
		int _sockFd;
		std::vector<struct pollfd> _pollFds;

		void handleConnection();
		void handleClient(size_t i);

	public:
		Server(int port);
		Server(const Server &other);
		Server& operator=(const Server &other);
		~Server();

		void runEventLoop();
};

Server::Server(int port) {
	this->_sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_sockFd < 0)
		throw std::runtime_error("socket failed");

	// TODO: set non-block

	// os belirtilen port ve adres kullanimdaysa hemen serbest birakmadigi
	// icin (TIME_WAIT state) reuse ile zorluyoruz. eger reuse olmasaydi ve
	// TIME_WAIT durumundaki port veya adrese baglanti kurulmaya calisilsaydi:
	// "Address already in use" hatasi alinacakti.
	int opt = 1;
	if (setsockopt(this->_sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("socketopt failed");
	if (setsockopt(this->_sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("socketopt failed");

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port); // big-endian (network byte order)

	if (bind(this->_sockFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind failed");

	// socket'i listen moduna alir, syn ve accept queuelari allocate edilir
	// kernel-space'te
	if (listen(this->_sockFd, 10) < 0)
		throw std::runtime_error("listen failed");

	// server fd'sine gelen eventlerin poll ile izlenmesini istiyoruz
	// ve bu yuzden pushluyoruz pollFds'e.
	struct pollfd pfd = {
		.fd = this->_sockFd,
		.events = POLLIN, // kernel'in bu fd icin yakalyacagi event
		.revents = 0 // kernelin dondugu eventler
	};
	this->_pollFds.push_back(pfd);
}

void Server::runEventLoop() {
	while (true) {
		// -1 yeni event'e kadar process'i sleep mode'a aliyor.
		printf("blocklancam.\n");

		int ready_count = poll(&_pollFds[0], _pollFds.size(), -1);

		printf("tetiklendiiiim.\n");

		if (ready_count < 0) // hata durumu
			break;
		else if (ready_count == 0) // timeout durumu
			continue;

		for (size_t i = 0; i < _pollFds.size(); ++i) {
			// herhangi bir event gelmediyse skip
			if (_pollFds[i].revents == 0)
				continue;

			// hata durumunda fd'yi kapatip listeden ucur
			if (_pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
				close(_pollFds[i].fd);
				_pollFds.erase(_pollFds.begin() + i);

				i--;
				continue;
			}

			// event'ten gelen veri varsa
			if (_pollFds[i].revents & POLLIN) {
				// fd, serverfd'ye esitse yeni connection
				if (_pollFds[i].fd == _sockFd)
					handleConnection();
				// eger degilse client yeni http istegi
				else
					handleClient(i);
			}
		}
	}
}

void Server::handleConnection() {
	
}

void Server::handleClient(size_t i) {

}

Server::~Server() {
	if (_sockFd >= 0)
		close(_sockFd);
}

int main() {
	Server s(8080);
	s.runEventLoop();

	return 0;
}