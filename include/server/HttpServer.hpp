#ifndef HTTPSERVER_H
# define HTTPSERVER_H

#include "Reactor.hpp"
#include <vector>

class HttpServer {
	private:
		Dispatcher _dispatcher;
		std::vector<Server*> _servers;

		HttpServer(const HttpServer&);
		HttpServer& operator=(const HttpServer&);
	public:
		HttpServer();
		~HttpServer();

		void add_server(Server* server);
		void run();
		void stop();
};

#endif
