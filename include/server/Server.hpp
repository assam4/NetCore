#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <cstddef>
#include <sys/types.h>
#include "Socket.hpp"
#include <stdint.h>

namespace http {
	namespace core  {

		/**
		 * @class Connection
		 * @brief Buffered wrapper over a connected client socket.
		 * @details Stores read/write buffers and exposes incremental I/O helpers.
		 *          Used by protocol parsers to consume input safely and progressively.
		 */
		class Connection {
			private:
				std::string  _read_buf;
				std::string  _write_buf;
				std::size_t  _write_offset;
				uint16_t _local_port;
				ClientSocket _socket;

				Connection(const Connection&);
				Connection& operator=(const Connection&);
			public:
				static const std::size_t MAX_READ_BUF = 8 * 1024 * 1024;

				explicit Connection(int fd, uint16_t _local_port);
				~Connection();

				ssize_t read_once();
				void append_write(const std::string& data);
				bool flush_write();
				const std::string& read_buffer() const;
				void consume_read(std::size_t n);
				bool has_pending_write() const;
				bool is_alive() const;
				int get_fd() const;
				uint16_t get_local_port() const;

				static Connection* make_connection(class ServerSocket& server);
		};

		/**
		 * @class Server
		 * @brief In-memory registry of listening sockets and active clients.
		 * @details Owns accepted connections and listener descriptors by fd.
		 *          Provides lookup and lifecycle operations for reactor handlers.
		 */
		class Server {
			private:
				std::map<int, ServerSocket*> _listens;
				std::map<int, Connection*> _clients;

				Server(const Server&);
				Server& operator=(const Server&);
			public:
				Server();
				~Server();

				void add_socket(ServerSocket* s);
				bool is_listener(int fd) const;
				ServerSocket* get_listener(int fd) const;
				Connection* accept_client(int fd);
				Connection* find_client(int fd) const;
				void remove_client(int fd);
				const std::map<int, ServerSocket*>& get_sockets() const;
				const std::map<int, Connection*>&   get_clients() const;
		};
	}
}
#endif
