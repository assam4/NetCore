#ifndef CONNECTION_HPP
# define CONNECTION_HPP

#include "ClientSocket.hpp"

class Connection {
	private:
		std::string _read_buf;
		std::string _write_buf;
		ClientSocket _socket;

		Connection(const Connection&);
		Connection& operator=(const Connection&);
	public:
		explicit Connection(int fd);
		~Connection();

		int read_once();
		void append_write(const std::string& data);
		bool flush_write();
		const std::string& read_buffer()  const;
		void consume_read(std::size_t n);
		bool has_pending_write() const;
		bool is_alive() const;

		static Connection* make_connection(class ServerSocket& server);
};

#endif
