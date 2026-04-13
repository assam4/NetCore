#ifndef REACTOR_HPP
# define REACTOR_HPP

# include <stdint.h>
# include <map>
# include "Server.hpp"

namespace http {
	namespace core {

		class HttpServer;

		/**
		 * @class AEventHandler
		 * @brief Base interface for all reactor event handlers.
		 * @details Provides a common contract for fd access and event processing.
		 *          Implementations define read/write/accept behavior per descriptor.
		 */
		class AEventHandler {
			public:
				virtual ~AEventHandler() {}
				virtual int get_fd() const = 0;
				virtual void handle_event(uint32_t events) = 0;
				virtual void clean_up() {}
		};

		/**
		 * @class Dispatcher
		 * @brief Reactor dispatcher that multiplexes I/O events.
		 * @details Owns the polling descriptor and the handler registry.
		 *          Registers/modifies/removes handlers and runs the event loop.
		 */
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
				void run();
				void stop();
		};


		/**
		 * @class AcceptHandler
		 * @brief Handler responsible for accepting new client connections.
		 * @details Reacts to readiness on a listening socket and creates clients.
		 *          Passes accepted connections to the server/dispatcher pipeline.
		 */
		class AcceptHandler : public AEventHandler {
			private:
				int _fd;
				Server& _server;
				Dispatcher& _dispatcher;
				HttpServer& _http_server;

				AcceptHandler(const AcceptHandler&);
				AcceptHandler& operator=(const AcceptHandler&);
			public:
				AcceptHandler(int fd, Server& srv, Dispatcher& Dispatcher, HttpServer& http_server);
				~AcceptHandler();

				int  get_fd() const;
				void handle_event(uint32_t events);
		};

		/**
		 * @class ConnectionHandler
		 * @brief Handler for per-client read/write lifecycle in the reactor.
		 * @details Processes socket events, tracks activity time, and performs cleanup.
		 *          Integrates protocol parsing with non-blocking connection management.
		 */
		class ConnectionHandler : public AEventHandler {
			private:
				Connection* _conn;
				Server& _server;
				Dispatcher& _dispatcher;
				HttpServer& _http_server;
				bool _half_closed;
				time_t _last_active;

				ConnectionHandler(const ConnectionHandler&);
				ConnectionHandler& operator=(const ConnectionHandler&);

				bool handle_read();
				bool handle_write();
				void clean_up();
			public:
				ConnectionHandler(Connection* conn, Server& server, Dispatcher& dispatcher, HttpServer& http_server);
				~ConnectionHandler();

				int get_fd() const;
				time_t last_active() const;
				void handle_event(uint32_t events);
		};

	}
}
#endif
