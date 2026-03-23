#ifndef HTTPSERVER_H
# define HTTPSERVER_H

#include "Reactor.hpp"
#include "configparser.hpp"
#include "virtualhost.hpp"
#include <vector>

class HttpServer {
	private:
		std::vector<http::core::VirtualHost> _virtual_hosts;
		Dispatcher _dispatcher;
		Server _server;
		static volatile sig_atomic_t _shutdown;

		HttpServer(const HttpServer&);
		HttpServer& operator=(const HttpServer&);

		static void signal_handler(int sig);
	public:
		HttpServer();
		~HttpServer();

		void init(const std::vector<http::config::parser::__server_row_data>& row_config);
		const http::core::VirtualHost* match_vhost(uint16_t port, const std::string& host_head);
		void run();
		void stop();
};

#endif
