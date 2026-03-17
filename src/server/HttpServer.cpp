#include "HttpServer.hpp"
#include <sys/epoll.h>

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

void HttpServer::run() { _dispatcher.run(); }

void HttpServer::stop() { _dispatcher.stop(); }
