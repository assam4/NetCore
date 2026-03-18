#include "HttpServer.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>

static int create_server_socket(int port) {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw std::runtime_error("socket() failed");

	int opt = 1;
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		close(fd);
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
	}
	// allows reuse even if port is in TIME_WAIT state
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		close(fd);
		throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port        = htons(port);

	if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
		close(fd);
		throw std::runtime_error("bind() failed (is port busy?)");
	}

	// THIS was missing — without listen() port is bound but not accepting
	if (::listen(fd, SOMAXCONN) < 0) {
		close(fd);
		throw std::runtime_error("listen() failed");
	}

	return fd;
}

int main() {
	try {
		int listen_fd = create_server_socket(2000);
		std::cout << "[Server] listening on port 2000\n";

		Server* srv = new Server();
		ServerSocket* ss = new ServerSocket(listen_fd);
		srv->add_socket(ss);

		HttpServer http;
		http.add_server(srv);
		http.run();
	} catch (const std::exception& ex) {
		std::cerr << "Fatal: " << ex.what() << "\n";
		return 1;
	}
	return 0;
}

