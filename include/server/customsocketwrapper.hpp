#ifndef CUSTOM_SOCKET_WRAPPER
# define CUSTOM_SOCKET_WRAPPER

# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <stdexcept>
# include <string>
# include <arpa/inet.h>

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

                int bind(uint32_t ip, uint16_t port) {
                    m_addr.sin_family = AF_INET;
                    m_addr.sin_port = htons(port);
                    m_addr.sin_addr.s_addr = htonl(ip);
                    return ::bind(m_fd, (sockaddr*)&m_addr, sizeof(m_addr));
                }

                int listen(int backlog) const {
                    return ::listen(m_fd, backlog);
                }

                CustomSocketWrapper accept() {
                    sockaddr_in client_addr;
                    socklen_t addrlen = sizeof(client_addr);
                    int client_fd = ::accept(m_fd, (sockaddr*)&client_addr, &addrlen);
                    if (client_fd == -1)
                        throw std::runtime_error("Failed to accept connection");
                    CustomSocketWrapper client_socket(client_fd, true);
                    client_socket.m_addr = client_addr;
                    return client_socket;
                }

                int connect(uint32_t ip, uint16_t port) {
                    m_addr.sin_family = AF_INET;
                    m_addr.sin_port = htons(port);
                    m_addr.sin_addr.s_addr = htonl(ip);
                    return ::connect(m_fd, (sockaddr*)&m_addr, sizeof(m_addr));
                }

                ssize_t send(const void *buf, size_t len, int flags) {
                    return ::send(m_fd, buf, len, flags);
                }

                ssize_t recv(void *buf, size_t len, int flags) {
                    return ::recv(m_fd, buf, len, flags);
                }

                int get_fd() const throw() { 
                    return m_fd;
                }

                sockaddr_in&    get_addr() {
                    return m_addr;
                }

                const sockaddr_in&  get_addr() const {
                    return m_addr;
                }

            private:
                explicit CustomSocketWrapper(int existing_fd, bool) : m_fd(existing_fd) {}
                CustomSocketWrapper(const CustomSocketWrapper&);
                CustomSocketWrapper& operator=(const CustomSocketWrapper&);
                sockaddr_in m_addr;
                int m_fd;

        };

    }
}

#endif