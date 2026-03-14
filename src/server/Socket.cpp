#include "Socket.hpp"
#include <unistd.h>

Socket::Socket(int fd) : _fd(fd), _alive(fd >= 0) {}

Socket::~Socket() {
	close_fd();
}

void Socket::close_fd() {
	if (_fd >= 0) {
		::close(_fd);
		_fd = -1;
	}
}

void Socket::invalidate() {
	_alive = false;
	close_fd();
}

int  Socket::get_fd()   const { return _fd; }
bool Socket::is_valid() const { return _alive && _fd >= 0; }
