#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

#include "Socket.hpp"

class ServerSocket : public Socket {
	private:
		ServerSocket(const ServerSocket&);
		ServerSocket& operator=(const ServerSocket&);
	public:
		ServerSocket(int fd);
		virtual ~ServerSocket();

		void listen(int backlog = 128);
		int accept_fd();
		void set_reuse_addr();
		virtual void make_non_blocking();
};

#endif
