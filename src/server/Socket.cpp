#include "Socket.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <iostream>

NetException::NetException(const std::string& msg) : message(msg + ": " + strerror(errno)) {}

NetException::~NetException() throw() {}

const char* NetException::what() const throw() {
	return message.c_str();
}

Socket::Socket(int fd) : _fd(fd), _alive(fd >= 0) {}

Socket::~Socket() {
	if (_fd >= 0)
		::close(_fd);
}

void Socket::invalidate() { _alive = false; }

int  Socket::get_fd() const { return _fd; }

bool Socket::is_valid() const { return _alive && _fd >= 0; }

void Socket::set_nonblocking() {
	int flags = ::fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
		throw NetException("fcntl F_GETFL failed");
	if (::fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw NetException("fcntl F_SETFL failed");
}

ClientSocket::ClientSocket(int fd) : Socket(fd) {
	if (fd >= 0) {
		set_nonblocking();
		set_nodelay();
	}
}

ClientSocket::~ClientSocket() {}

void ClientSocket::set_nodelay() {
	int on = 1;
	if (::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0)
		std::cerr << "setsockopt TCP_NODELAY failed\n";
}

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

ServerSocket::ServerSocket(int fd) : Socket(fd) {
	if (fd >= 0) {
		set_nonblocking();
		set_reuse_addr();
	}
}

ServerSocket::~ServerSocket() {}

void ServerSocket::set_reuse_addr() {
	int opt = 1;
	if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw NetException("Reuse option failed");
}

void ServerSocket::listen(int backlog) {
	if (::listen(_fd, backlog) < 0) {
		throw NetException("Socket listen failed");
	}
}

int ServerSocket::accept_fd() {
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);
	return ::accept(get_fd(), reinterpret_cast<sockaddr*>(&client_addr), &len);
}
