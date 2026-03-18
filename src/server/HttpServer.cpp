#include "HttpServer.hpp"
#include <sys/epoll.h>
#include <csignal>

volatile sig_atomic_t HttpServer::_shutdown = 0;

void HttpServer::signal_handler(int sig) {
	static_cast<void>(sig);
	_shutdown = 1;
}

HttpServer::HttpServer() {}

HttpServer::~HttpServer() {
	for (std::size_t i = 0; i < _servers.size(); ++i)
		delete _servers[i];
}

void HttpServer::add_server(Server* server) {
	_servers.push_back(server);
	const std::map<int, ServerSocket*>& sockets = server->get_sockets();
	for (std::map<int, ServerSocket*>::const_iterator it = sockets.begin(); it != sockets.end(); ++it) {
		AcceptHandler *ah = new AcceptHandler(it->first, *server, _dispatcher);
		_dispatcher.register_handler(ah, EPOLLIN);
	}
}

void HttpServer::run() {
	signal(SIGINT, HttpServer::signal_handler);
	signal(SIGTERM, HttpServer::signal_handler);
	signal(SIGPIPE, SIG_IGN);
	_dispatcher.run(_shutdown);
}

void HttpServer::stop() { _dispatcher.stop(); }
