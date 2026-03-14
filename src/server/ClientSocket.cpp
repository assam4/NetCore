#include "ClientSocket.hpp"
#include "NetExceptions.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

ClientSocket::ClientSocket(int fd) : Socket(fd) {
	if (fd >= 0)
		make_non_blocking();
}

ClientSocket::~ClientSocket() {}

std::string ClientSocket::receive() {
	if (!is_valid())
		return "";
	char buf[BUFFER_SIZE];
	ssize_t n = ::recv(get_fd(), buf, sizeof(buf), 0);
	if (n <= 0) {
		invalidate();
		return "";
	}
	return std::string(buf, static_cast<std::size_t>(n));
}

ssize_t ClientSocket::send_raw(const char* data, std::size_t len) {
	if (!is_valid())
		return -1;
	ssize_t sent = ::send(get_fd(), data, len, 0);
	if (sent < 0) {
		invalidate();
		return -1;
	}
	return sent;
}

void ClientSocket::make_non_blocking() {
	int fd = get_fd();
	int flags = ::fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		throw StrictError(std::string("fcntl F_GETFL: ") + strerror(errno));
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw StrictError(std::string("fcntl F_SETFL: ") + strerror(errno));
	int on = 1;
	if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0)
		std::cerr << "Warning: TCP_NODELAY: " << strerror(errno) << std::endl;
}
