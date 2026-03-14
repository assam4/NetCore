#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "Socket.hpp"
#include <string>

class ClientSocket : public Socket {
	private:
		static const int BUFFER_SIZE = 4096;

		ClientSocket(const ClientSocket&);
		ClientSocket& operator=(const ClientSocket&);
	public:
		explicit ClientSocket(int fd = -1);
		virtual ~ClientSocket();

		std::string		receive();
		ssize_t			send_raw(const char* data, std::size_t len);
		virtual void	make_non_blocking();
};

#endif
