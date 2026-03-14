#include "Connection.hpp"
#include "ServerSocket.hpp"
#include "NetExceptions.hpp"
#include <iostream>

Connection::Connection(int fd) : _socket(fd) {}

Connection::~Connection() {}

int Connection::read_once() {
	if (!_socket.is_valid())
		return -1;
	std::string chunk = _socket.receive();
	if (chunk.empty())
		return _socket.is_valid() ? 0 : -1;
	_read_buf += chunk;
	return static_cast<int>(chunk.size());
}

void Connection::append_write(const std::string& data) {
	_write_buf += data;
}

bool Connection::flush_write() {
	if (_write_buf.empty())
		return true;
	std::size_t total = 0;
	while (total < _write_buf.size()) {
		ssize_t sent = _socket.send_raw(
			_write_buf.c_str() + total,
			_write_buf.size() - total
		);
		if (sent < 0) {
			return false;
		}
		total += static_cast<std::size_t>(sent);
	}
	_write_buf.erase(0, total);
	return _write_buf.empty();
}

const std::string& Connection::read_buffer() const {
	return _read_buf;
}

void Connection::consume_read(std::size_t n) {
	if (n >= _read_buf.size())
		_read_buf.clear();
	else
		_read_buf.erase(0, n);
}

bool Connection::has_pending_write() const {
	return !_write_buf.empty();
}

bool Connection::is_alive() const {
	return _socket.is_valid();
}

Connection* Connection::make_connection(ServerSocket& server) {
	try {
		int fd = server.accept_fd();
		return new Connection(fd);
	} catch (const WeakError& e) {
		std::cerr << "accept failed: " << e.what() << std::endl;
		return NULL;
	}
}
