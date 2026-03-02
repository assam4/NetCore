#ifndef CUSTOM_SOCKET_WRAPPER
# define CUSTOM_SOCKET_WRAPPER

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <stdexcept>

namespace http {
    namespace utils {
        class   CustomSocketWrapper {
            public:
                CustomSocketWrapper(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0): m_fd(socket(domain, type, protocol)) {
                    if (m_fd == -1)
                        throw std::runtime_error("Failed to create socket");
                }
                ~CustomSocketWrapper() {
                    if (m_fd != -1)
                        close(m_fd);
                }

                int bind(const sockaddr *addr, socklen_t len) const {
                    return ::bind(m_fd, addr, len);
                }

                int listen(int backlog) const {
                    return ::listen(m_fd, backlog);
                }

                int accept(sockaddr* addr, socklen_t *addrlen) {
                    return ::accept(m_fd, addr, addrlen);
                }

                int connect(const sockaddr* addr, socklen_t addrlen) {
                    return ::connect(m_fd, addr, addrlen);
                }

                int get_fd() const throw() { 
                    return m_fd;
                }
            private:
                CustomSocketWrapper(const CustomSocketWrapper&);
                CustomSocketWrapper& operator=(const CustomSocketWrapper&);
                int m_fd;
        };

    }
}

#endif