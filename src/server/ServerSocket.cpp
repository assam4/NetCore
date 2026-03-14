#include "ServerSocket.hpp"
#include "NetExceptions.hpp"
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fcntl.h>

ServerSocket::ServerSocket(int fd) : Socket(fd) {
	make_non_blocking();
}

ServerSocket::~ServerSocket() {}

void ServerSocket::set_reuse_addr() {
	int opt = 1;
	if (::setsockopt(get_fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw StrictError(strerror(errno));
}

void ServerSocket::make_non_blocking() {
	int flags = ::fcntl(get_fd(), F_GETFL, 0);
	if (flags < 0)
		throw StrictError(std::string("fcntl F_GETFL: ") + strerror(errno));
	if (::fcntl(get_fd(), F_SETFL, flags | O_NONBLOCK) < 0)
		throw StrictError(std::string("fcntl F_SETFL: ") + strerror(errno));
}

void ServerSocket::listen(int backlog) {
	if (::listen(get_fd(), backlog) < 0) {
		throw StrictError(strerror(errno));
	}
}

int ServerSocket::accept_fd() {
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);
	int client_fd = ::accept(get_fd(), reinterpret_cast<sockaddr*>(&client_addr), &len);
	if (client_fd < 0)
		throw WeakError(std::string("accept: ") + strerror(errno));
	return client_fd;
}
