#ifndef REACTOR_HPP
# define REACTOR_HPP

# include <cstdint>
# include <map>
# include "Server.hpp"

class IEventHandler {
	public:
		virtual ~IEventHandler() {}
		virtual int get_fd() const = 0;
		virtual void handle_event(uint32_t events) = 0;
};

class Dispatcher {
	private:
		int _epfd;
		bool _running;
		std::map<int, IEventHandler*> _handlers;

		Dispatcher(const Dispatcher&);
		Dispatcher& operator=(const Dispatcher&);
	public:
		Dispatcher();
		~Dispatcher();

		void register_handler(IEventHandler* h, uint32_t events);
		void remove_handler(IEventHandler* h);
		void modify_handler(IEventHandler* h, uint32_t events);

		void dispatch();
		void run();
		void stop();
};


class AcceptHandler : public IEventHandler {
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

class ConnectionHandler : public IEventHandler {
	private:
		Connection* _conn;
		Server& _server;
		Dispatcher& _dispatcher;
		bool _half_closed;

		ConnectionHandler(const ConnectionHandler&);
		ConnectionHandler& operator=(const ConnectionHandler&);

		bool handle_read();
		bool handle_write();
		void clean_up();
	public:
		ConnectionHandler(Connection* conn, Server& server, Dispatcher& dispatcher);
		~ConnectionHandler();

		int get_fd() const;
		void handle_event(uint32_t events);
};

#endif
