#include "Reactor.hpp"
#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <cerrno>

namespace http {
	namespace core {

		extern volatile sig_atomic_t g_shutdown;

		Dispatcher::Dispatcher() : _epfd(-1), _running(false) {
			_epfd = epoll_create1(0);
			if (_epfd < 0)
				throw std::runtime_error("Epoll creation failed");
		}

		Dispatcher::~Dispatcher() {
			 for (std::map<int, AEventHandler*>::iterator it = _handlers.begin(); it != _handlers.end(); ++it)
				delete it->second;
			_handlers.clear();
			if (_epfd >= 0)
				::close(_epfd);
		}

		void Dispatcher::check_timeouts() {
			time_t now = time(NULL);
			std::vector<AEventHandler*> to_close;
			for(std::map<int, AEventHandler*>::const_iterator it = _handlers.begin(); it != _handlers.end(); ++it) {
				ConnectionHandler* ch = dynamic_cast<ConnectionHandler*>(it->second);
				if (ch && now - ch->last_active() > CONN_TIMEOUT)
					to_close.push_back(ch);
			}
			for (std::vector<AEventHandler*>::iterator it = to_close.begin(); it != to_close.end(); ++it)
				(*it)->clean_up();
		}

		void Dispatcher::register_handler(AEventHandler* h, uint32_t events) {
			epoll_event ev;
			ev.events = events;
			ev.data.fd = h->get_fd();
			if (::epoll_ctl(_epfd, EPOLL_CTL_ADD, h->get_fd(), &ev) < 0)
				throw NetException("Epoll_ctl add failed");
			_handlers[h->get_fd()] = h;
		}

		void Dispatcher::remove_handler(AEventHandler* h) {
			::epoll_ctl(_epfd, EPOLL_CTL_DEL, h->get_fd(), NULL);
			_handlers.erase(h->get_fd());
		}

		void Dispatcher::modify_handler(AEventHandler* h, uint32_t events) {
			epoll_event ev;
			ev.events = events;
			ev.data.fd = h->get_fd();
			if (::epoll_ctl(_epfd, EPOLL_CTL_MOD, h->get_fd(), &ev) < 0)
				throw NetException("epoll_ctl MOD failed");
		}

		void Dispatcher::dispatch() {
			static time_t last_timeout_check = time(NULL);
			epoll_event events[MAX_EVENTS];
			int n = ::epoll_wait(_epfd, events, MAX_EVENTS, EPOLL_TIMEOUT);
			if (n < 0) {
				if (errno == EINTR)
					return ;
				throw NetException("epoll_wait failed");
			}
			time_t now = time(NULL);
			if (now - last_timeout_check >= 5) {
				check_timeouts();
				last_timeout_check = now;
			}
			for (int i = 0; i < n; ++i) {
				int fd = events[i].data.fd;
				std::map<int, AEventHandler*>::iterator it = _handlers.find(fd);
				if (it != _handlers.end())
					it->second->handle_event(events[i].events);
			}
		}

		void Dispatcher::stop() { _running = false; }

		void Dispatcher::run(volatile sig_atomic_t& shutdown) {
			_running = true;
			while (_running && !shutdown) {
				try {
					dispatch();
				} catch (const NetException& e) {
					std::cerr << "[Dispatcher] error: " << e.what() << "\n";
				} catch (const std::exception& e) {
					std::cerr << "[Dispatcher] fatal: " << e.what() << "\n";
					break;
				}
			}
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
			: _conn(conn), _server(srv), _dispatcher(disp), _half_closed(false), _last_active(time(NULL)) {}

		ConnectionHandler::~ConnectionHandler() {}

		bool ConnectionHandler::handle_read() {
			_last_active = time(NULL);
			ssize_t n = _conn->read_once();
			if (n < 0)
				return false;
			if (n == 0)
				return true;
			_conn->consume_read(_conn->read_buffer().size());
			 std::string response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 14\r\n"
				"Content-Type: text/plain\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				"Hello, World!\n";
			_conn->append_write(response);
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

		time_t ConnectionHandler::last_active() const { return _last_active; }

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
			if (!should_close && events & EPOLLOUT)
				should_close = (handle_write() == false);
			if (!should_close && _half_closed && !_conn->has_pending_write())
				should_close = true;
			if (should_close)
				clean_up();
		}

	}
}
