#include "Reactor.hpp"
#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

static const int MAX_EVENTS = 64;



Dispatcher::Dispatcher() : _epfd(-1), _running(false) {
	_epfd = epoll_create1(0);
	if (_epfd < 0)
		std::runtime_error("Epoll creation failed");
}

Dispatcher::~Dispatcher() {
	if (_epfd >= 0)
		::close(_epfd);
}

void Dispatcher::register_handler(IEventHandler* h, uint32_t events) {
	epoll_event ev;
	ev.events = events;
	ev.data.fd = h->get_fd();
	if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, h->get_fd(), &ev) < 0)
		throw NetException("Epoll_ctl add failed");
	_handlers[h->get_fd()] = h;
}

void Dispatcher::remove_handler(IEventHandler* h) {
	::epoll_ctl(_epfd, EPOLL_CTL_DEL, h->get_fd(), NULL);
	_handlers.erase(h->get_fd());
}

void Dispatcher::modify_handler(IEventHandler* h, uint32_t events) {
	epoll_event ev;
	ev.events = events;
	ev.data.fd = h->get_fd();
	if (::epoll_ctl(_epfd, EPOLL_CTL_MOD, h->get_fd(), &ev) < 0)
		throw NetException("epoll_ctl MOD failed");
}


void Dispatcher::dispatch() {
	epoll_event events[MAX_EVENTS];
	int n = ::epoll_wait(_epfd, events, MAX_EVENTS, -1);
	if (n < 0) {
		if (errno == EINTR)
			return ;
		throw NetException("epoll_wait failed");
	}
	for (int i = 0; i < n; ++i) {
		int fd = events->data.fd;
		std::map<int, IEventHandler*>::iterator it = _handlers.find(fd);
		if (it != _handlers.end())
			it->second->handle_event(events[i].events);
	}
}

void Dispatcher::stop() { _running = false; }

void Dispatcher::run() {
	_running = true;
	while(_running)
		dispatch();
}

AcceptHandler::AcceptHandler(int fd, Server& server, Dispatcher& dispatcher) :
	_fd(fd), _server(server), _dispatcher(dispatcher) {}

AcceptHandler::~AcceptHandler() {}

int AcceptHandler::get_fd() const { return _fd; }

void AcceptHandler::handle_event(uint32_t events) {
	if (events & (EPOLLERR | EPOLLHUP)) {
		std::cerr << "[AcceptHandler] error on listener " << _fd << "\n";
		return;
	}
	if (!(events & EPOLLIN))
		return ;
	Connection * conn = _server.accept_client(_fd);
	if (!conn)
		return ;
	ConnectionHandler *h = new ConnectionHandler(conn, _server, _dispatcher);
	_dispatcher.register_handler(h, (EPOLLIN | EPOLLRDHUP));
}

ConnectionHandler::ConnectionHandler(Connection* conn, Server& srv, Dispatcher& disp)
	: _conn(conn), _server(srv), _dispatcher(disp), _half_closed(false) {}

ConnectionHandler::~ConnectionHandler() {}

bool ConnectionHandler::handle_read() {
	ssize_t n = _conn->read_once();
	if (n < 0)
		return false;
	if (n == 0)
		return true;
	_conn->consume_read(_conn->read_buffer().size());
	 std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 13\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Hello, World!";
	_conn->append_write(_conn->read_buffer());
	_dispatcher.modify_handler(this, EPOLLOUT | EPOLLRDHUP);
	return true;
}

bool ConnectionHandler::handle_write() {
	bool done = _conn->flush_write();
	if (!_conn->is_alive())
		return false;
	if (done) {
		if (_half_closed)
			return false;
		else
			_dispatcher.modify_handler(this, EPOLLIN | EPOLLRDHUP);
	}
	return true;
}

void ConnectionHandler::clean_up() {
	int fd = _conn->get_fd();
	_dispatcher.remove_handler(this);
	_server.remove_client(fd);
	delete this;
}

int ConnectionHandler::get_fd() const { return _conn->get_fd(); }

void ConnectionHandler::handle_event(uint32_t events) {
	if (events & (EPOLLERR | EPOLLHUP)) {
		clean_up();
		return ;
	}
	if (events & EPOLLRDHUP)
		_half_closed = true;
	bool should_close = false;
	if (events & EPOLLIN)
		should_close = (handle_read() == false);
	if (events & EPOLLOUT)
		should_close = (handle_write() == false);
	if (!should_close && _half_closed && !_conn->has_pending_write())
		should_close = true;
	if (should_close)
		clean_up();
}
