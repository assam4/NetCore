#include "Server.hpp"
#include <iostream>

Connection::Connection(int fd) : _write_offset(0), _socket(fd) {}

Connection::~Connection() {}

ssize_t Connection::read_once() {
	if (!_socket.is_valid())
		return -1;
	if (_read_buf.size() >= MAX_READ_BUF) {
		std::cerr << "[Connection] read_once: read buffer full (fd " << _socket.get_fd() << ")\n";
		return -1;
	}
	std::string chunk = _socket.receive();
	if (chunk.empty())
		return _socket.is_valid() ? 0 : -1;

	_read_buf += chunk;
	return static_cast<ssize_t>(chunk.size());
}

void Connection::append_write(const std::string& data) {
	_write_buf += data;
}

bool Connection::flush_write() {
	if (_write_offset >= _write_buf.size())
		return true;
	ssize_t sent = _socket.send_raw(_write_buf.c_str() + _write_offset, _write_buf.size() - _write_offset);
	if (sent < 0)
		return false;
	_write_offset += static_cast<std::size_t>(sent);
	if (_write_offset >= _write_buf.size()) {
		_write_buf.clear();
		_write_offset = 0;
		return true;
	}
	return false;
}

const std::string& Connection::read_buffer() const { return _read_buf; }

void Connection::consume_read(std::size_t n) {
	if (n >= _read_buf.size())
		_read_buf.clear();
	else
		_read_buf.erase(0, n);
}

bool Connection::has_pending_write() const {
	return _write_offset < _write_buf.size();
}

bool Connection::is_alive() const {
	return _socket.is_valid();
}

int Connection::get_fd() const {
	return _socket.get_fd();
}

Connection* Connection::make_connection(ServerSocket& server) {
	int fd = server.accept_fd();
	if (fd < 0)
		return NULL;
	return new Connection(fd);
}

Server::Server() {}

Server::~Server() {
	for (std::map<int, ServerSocket*>::iterator it = _listens.begin(); it != _listens.end(); ++it)
		delete it->second;
	for (std::map<int, Connection*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
}

void Server::add_socket(ServerSocket* s) {
	if (!s) {
		std::cerr << "[Server] add_socket: NULL socket\n";
		return;
	}
	int fd = s->get_fd();
	if (_listens.count(fd)) {
		std::cerr << "[Server] add_socket: fd " << fd << " already registered\n";
		return;
	}
	s->listen();
	_listens[fd] = s;
}

ServerSocket* Server::get_listener(int fd) const {
	std::map<int, ServerSocket*>::const_iterator it = _listens.find(fd);
	if (it == _listens.end())
		return NULL;
	return it->second;
}

bool Server::is_listener(int fd) const {
	return get_listener(fd) != NULL;
}

Connection* Server::accept_client(int fd) {
	ServerSocket* listener = get_listener(fd);
	if (!listener)
		return NULL;
	Connection* conn = Connection::make_connection(*listener);
	if (!conn)
		return NULL;
	_clients[conn->get_fd()] = conn;
	return conn;
}

Connection* Server::find_client(int fd) const {
	std::map<int, Connection*>::const_iterator it = _clients.find(fd);
	if (it == _clients.end())
		return NULL;
	return it->second;
}

void Server::remove_client(int fd) {
	std::map<int, Connection*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return ;
	delete it->second;
	_clients.erase(it);
}

const std::map<int, ServerSocket*>& Server::get_sockets() const { return _listens; }

const std::map<int, Connection*>& Server::get_clients() const { return _clients; }
