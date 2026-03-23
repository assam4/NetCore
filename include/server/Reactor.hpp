#ifndef REACTOR_HPP
# define REACTOR_HPP

# include <stdint.h>
# include <csignal>
# include <map>
# include "Server.hpp"

namespace http {
	namespace core {
		
		class AEventHandler {
			public:
				virtual ~AEventHandler() {}
				virtual int get_fd() const = 0;
				virtual void handle_event(uint32_t events) = 0;
				virtual void clean_up() {}
		};

		class Dispatcher {
			private:
				static const int CONN_TIMEOUT = 30;
				static const int MAX_EVENTS = 1024;
				static const int EPOLL_TIMEOUT = 5000;

				int _epfd;
				bool _running;
				std::map<int, AEventHandler*> _handlers;

				Dispatcher(const Dispatcher&);
				Dispatcher& operator=(const Dispatcher&);

				void check_timeouts();
			public:
				Dispatcher();
				~Dispatcher();

				void register_handler(AEventHandler* h, uint32_t events);
				void remove_handler(AEventHandler* h);
				void modify_handler(AEventHandler* h, uint32_t events);

				void dispatch();
				void run(volatile sig_atomic_t& shutdown);
				void stop();
		};


		class AcceptHandler : public AEventHandler {
			private:
				int _fd;
				Server& _server;
				Dispatcher& _dispatcher;

				AcceptHandler(const AcceptHandler&);
				AcceptHandler& operator=(const AcceptHandler&);
			public:
				AcceptHandler(int fd, Server& srv, Dispatcher& Dispatcher);
				~AcceptHandler();

				int  get_fd() const;
				void handle_event(uint32_t events);
		};

		class ConnectionHandler : public AEventHandler {
			private:
				Connection* _conn;
				Server& _server;
				Dispatcher& _dispatcher;
				bool _half_closed;
				time_t _last_active;

				ConnectionHandler(const ConnectionHandler&);
				ConnectionHandler& operator=(const ConnectionHandler&);

				bool handle_read();
				bool handle_write();
				void clean_up();
			public:
				ConnectionHandler(Connection* conn, Server& server, Dispatcher& dispatcher);
				~ConnectionHandler();

				int get_fd() const;
				time_t last_active() const;
				void handle_event(uint32_t events);
		};

	}
}
#endif
