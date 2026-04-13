#if defined(__linux__)
	# include <sys/epoll.h>
	# include <ctime>
#elif defined(__APPLE__) || defined(__FreeBSD__)
	# include <sys/event.h>
	# include <sys/time.h>
#else
	#error "Unsupported platform"
#endif
#include "Reactor.hpp"
#include "HttpServer.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_transaction.hpp"
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cerrno>
#include "SignalHandler.hpp"

namespace http {
	namespace core {

		Dispatcher::Dispatcher() : _epfd(-1), _running(false) {
			#if defined(__linux__)
				_epfd = epoll_create1(0);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				_epfd = kqueue();
			#endif
			if (_epfd < 0)
				throw std::runtime_error("Server error: Failed to create event queue.\n");
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
			int	status = 0;
			#if defined(__linux__)
				epoll_event ev;
				ev.events = events;
				ev.data.fd = h->get_fd();
				status = ::epoll_ctl(_epfd, EPOLL_CTL_ADD, h->get_fd(), &ev);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				(void)events;
				struct kevent	kev;
				EV_SET(&kev, h->get_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0 , NULL);
				status = kevent(_epfd, &kev, 1, NULL, 0, NULL);
			#endif
			if (status < 0)
				throw NetException("Server error: Event add failed.\n");
			_handlers[h->get_fd()] = h;
		}

		void Dispatcher::remove_handler(AEventHandler* h) {
			int	status = 0;
			#if defined(__linux__)
				status = ::epoll_ctl(_epfd, EPOLL_CTL_DEL, h->get_fd(), NULL);
				if (status < 0)
					throw NetException("Server error: Event remove failed.\n");
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				struct kevent	kev[2];
				EV_SET(&kev[0], h->get_fd(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
				EV_SET(&kev[1], h->get_fd(), EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
				status = kevent(_epfd, kev, 2, NULL, 0, NULL);
			#endif
			_handlers.erase(h->get_fd());
		}

		void Dispatcher::modify_handler(AEventHandler* h, uint32_t events) {
			int	status = 0;
			#if defined(__linux__)
				epoll_event ev;
				ev.events = events;
				ev.data.fd = h->get_fd();
				status = ::epoll_ctl(_epfd, EPOLL_CTL_MOD, h->get_fd(), &ev);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				struct kevent kev;
				if ((int)events == EVFILT_WRITE) {
					EV_SET(&kev, h->get_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				} else {
					EV_SET(&kev, h->get_fd(), EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
				}
				status = kevent(_epfd, &kev, 1, NULL, 0, NULL);
			#endif
			if (status < 0)
				throw NetException("Server error: modify handler(MOD) failed.\n");
		}

		void Dispatcher::dispatch() {
			static time_t last_timeout_check = time(NULL);
			#if defined(__linux__)
				epoll_event events[MAX_EVENTS];
				int	n = ::epoll_wait(_epfd, events, MAX_EVENTS, EPOLL_TIMEOUT);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				struct kevent	events[MAX_EVENTS];
				struct timespec	timeout;
				timeout.tv_sec = EPOLL_TIMEOUT / 1000;
				timeout.tv_nsec = (EPOLL_TIMEOUT % 1000) * 1000000L;
				int	n = kevent(_epfd, NULL, 0, events, MAX_EVENTS, &timeout);
			#endif
			if (n < 0) {
				if (errno == EINTR)
					return ;
				throw NetException("Server error: Waiting event failed.\n");
			}
			time_t now = time(NULL);
			if (now - last_timeout_check >= 5) {
				check_timeouts();
				last_timeout_check = now;
			}
			#if defined(__linux__)
				for (int i = 0; i < n; ++i) {
					int fd = events[i].data.fd;
					std::map<int, AEventHandler*>::iterator it = _handlers.find(fd);
					if (it != _handlers.end())
						it->second->handle_event(events[i].events);
				}
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				for (int i = 0; i < n; ++i) {
					int	fd = events[i].ident;
					std::map<int, AEventHandler*>::iterator it = _handlers.find(fd);
					if (it != _handlers.end())
						it->second->handle_event(events[i].filter);
				}
			#endif
		}

		void Dispatcher::stop() { _running = false; }

		void Dispatcher::run() {
			_running = true;
			while (_running && !SignalHandler::get_shutdown()) {
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

		AcceptHandler::AcceptHandler(int fd, Server& server, Dispatcher& dispatcher, HttpServer& http_server) :
			_fd(fd), _server(server), _dispatcher(dispatcher), _http_server(http_server) {}

		AcceptHandler::~AcceptHandler() {}

		int AcceptHandler::get_fd() const { return _fd; }

		void AcceptHandler::handle_event(uint32_t events) {
			#if defined(__linux__)
				if (events & (EPOLLERR | EPOLLHUP)) {
					std::cerr << "[AcceptHandler] error on listener " << _fd << "\n";
					return;
				}
				if (!(events & EPOLLIN))
					return ;
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				if (static_cast<int>(events) & (EV_ERROR | EV_EOF)) {
					std::cerr << "[AcceptHandler] error on listener " << _fd << "\n";
					return;
				}
				if (static_cast<int>(events) != EVFILT_READ)
					return;
			#endif
			Connection	*conn = _server.accept_client(_fd);
			if (!conn)
				return ;
			ConnectionHandler *h = new ConnectionHandler(conn, _server, _dispatcher, _http_server);
			#if defined(__linux__)
				_dispatcher.register_handler(h, (EPOLLIN | EPOLLRDHUP));
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				_dispatcher.register_handler(h, (EVFILT_READ));
			#endif
		}

ConnectionHandler::ConnectionHandler(Connection* conn, Server& srv, Dispatcher& disp, HttpServer& http_server)
				: _conn(conn), _server(srv), _dispatcher(disp), _http_server(http_server), _half_closed(false), _last_active(time(NULL)) {}

		ConnectionHandler::~ConnectionHandler() {}

		bool ConnectionHandler::handle_read() {
			_last_active = time(NULL);
			ssize_t n = _conn->read_once();
			if (n < 0)
				return false;
			if (n == 0)
				return true;
			std::pair<types::HttpStatus, Request> status_req = Request::parse_message(*_conn);
			const Request& req = status_req.second;
			std::string host_header;
			std::map<std::string, std::vector<std::string> >::const_iterator host_it = req.headers.header_map.find("host");
			if (host_it != req.headers.header_map.end() && !host_it->second.empty())
				host_header = host_it->second.front();
			size_t colon_pos = host_header.find(':');
			if (colon_pos != std::string::npos)
				host_header.erase(colon_pos);
			const http::core::VirtualHost* vhost = _http_server.find_vhost(_conn->get_local_port(), host_header);
			if (!vhost)
				return false;
			const types::__location& location = HttpTransaction::get_best_location(*vhost, req.start_line.uri);
			Response::_http_response response = Response::make_response(status_req, location);
			_conn->append_write(Response::serialize(response));
			#if defined(__linux__)
				_dispatcher.modify_handler(this, EPOLLOUT | EPOLLRDHUP);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				_dispatcher.modify_handler(this, EVFILT_WRITE);
			#endif
			return true;
		}

		bool ConnectionHandler::handle_write() {
			bool done = _conn->flush_write();
			if (!_conn->is_alive())
				return false;
			if (done) {
				if (_half_closed)
					return false;
				else {
					#if defined(__linux__)
						_dispatcher.modify_handler(this, EPOLLIN | EPOLLRDHUP);
					#elif defined(__APPLE__) || defined(__FreeBSD__)
						_dispatcher.modify_handler(this, EVFILT_READ);
					#endif
				}
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
			#if defined(__linux__)
				if (events & (EPOLLERR | EPOLLHUP)) {
					clean_up();
					return ;
				}
				if (events & EPOLLRDHUP)
					_half_closed = true;
				bool	should_close = false;
				if (events & EPOLLIN)
					should_close = (handle_read() == false);
				if (!should_close && events & EPOLLOUT)
					should_close = (handle_write() == false);
			#elif defined(__APPLE__) || defined(__FreeBSD__)
				if (events & (EV_ERROR | EV_EOF)) {
					clean_up();
					return;
				}
				bool should_close = false;
				if ((int)events == EVFILT_READ)
					should_close = (handle_read() == false);
				if (!should_close && (int)events == EVFILT_WRITE)
					should_close = (handle_write() == false);
			#endif
			if (!should_close && _half_closed && !_conn->has_pending_write())
				should_close = true;
			if (should_close)
				clean_up();
		}

	}
}
