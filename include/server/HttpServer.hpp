#ifndef HTTPSERVER_H
# define HTTPSERVER_H

#include "Reactor.hpp"
#include "configparser.hpp"
#include "virtualhost.hpp"
#include <vector>

namespace http {
	namespace core {

		/**
		 * @class HttpServer
		 * @brief Top-level HTTP server runtime orchestrator.
		 * @details Initializes virtual hosts, server sockets, and reactor dispatcher.
		 *          Coordinates startup, host matching, run loop, and graceful stop.
		 */
		class HttpServer {
			private:
				std::vector<VirtualHost> _virtual_hosts;
				Dispatcher _dispatcher;
				Server _server;

				HttpServer(const HttpServer&);
				HttpServer& operator=(const HttpServer&);
			public:
				HttpServer();
				~HttpServer();

				void init(const std::vector<http::config::parser::__server_row_data>& row_config);
				const VirtualHost* find_vhost(uint16_t port, const std::string& host_head);
				void run();
				void stop();
		};
	}
}

#endif
