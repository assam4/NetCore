#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <string>
# include <sys/socket.h>
# include <exception>

namespace http {
	namespace core {

		class NetException : public std::exception {
			private:
				std::string message;
			public:
				explicit NetException(const std::string& msg);
				virtual ~NetException() throw();
				virtual const char* what() const throw();
		};

		class Socket {
			private:
				Socket(const Socket&);
				Socket& operator=(const Socket&);
			protected:
				int _fd;
				bool _alive;

				explicit Socket(int fd);
				virtual ~Socket();

				void set_nonblocking();
			public:
				int get_fd() const;
				bool is_valid() const;
				void invalidate();

		};

		class ClientSocket : public Socket {
			private:
				static const int BUFFER_SIZE = 4096;

				ClientSocket(const ClientSocket&);
				ClientSocket& operator=(const ClientSocket&);

				void set_nodelay();
			public:
				explicit ClientSocket(int fd);
				virtual ~ClientSocket();

				std::string receive();
				ssize_t send_raw(const char* data, std::size_t len);
		};

		class ServerSocket : public Socket {
			private:
				ServerSocket(const ServerSocket&);
				ServerSocket& operator=(const ServerSocket&);

				void set_reuse_addr();
			public:
				ServerSocket(int fd);
				virtual ~ServerSocket();

				void listen(int backlog = SOMAXCONN);
				int accept_fd();
		};
	}
}

#endif
