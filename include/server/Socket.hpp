#ifndef SOCKET_HPP
#define SOCKET_HPP

class Socket {
	private:
		int  _fd;
		bool _alive;

		Socket(const Socket&);
		Socket& operator=(const Socket&);
	protected:
		explicit Socket(int fd);
		virtual ~Socket();

		void close_fd();
	public:
		int  get_fd() const;
		bool is_valid() const;
		void invalidate();

		virtual void make_non_blocking() = 0;
};

#endif
