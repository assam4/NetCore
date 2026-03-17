#include "HttpServer.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <assert.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

static int create_server_socket(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(fd);
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        throw std::runtime_error("bind() failed (is port busy?)");
    }

    return fd;
}

static int get_port_from_fd(int fd) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) < 0)
        return -1;
    return ntohs(addr.sin_port);
}

static std::string http_request(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("client socket failed");

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        throw std::runtime_error("connect() failed");
    }

    const char* request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    ssize_t sent = ::send(fd, request, std::strlen(request), 0);
    if (sent <= 0) {
        close(fd);
        throw std::runtime_error("send() failed");
    }

    std::string response;
    char buf[1024];
    ssize_t n;
    while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
        response.append(buf, n);
    }
    close(fd);
    return response;
}

int main() {
    try {
        int listen_fd = create_server_socket(2000);
        int port = get_port_from_fd(listen_fd);
        assert(port > 0);

        Server* srv = new Server();
        ServerSocket* ss = new ServerSocket(listen_fd);
        srv->add_socket(ss);

        HttpServer http;
        http.add_server(srv);

        std::thread server_thread([&http](){
            http.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        bool all_ok = true;
        for (int i = 0; i < 5; ++i) {
            try {
                std::string response = http_request(port);
                bool ok = response.find("Hello, world!") != std::string::npos &&
                          response.find("HTTP/1.1 200 OK") != std::string::npos;
                std::cout << "[test " << i << "] response size=" << response.size() << " ok=" << ok << "\n";
                all_ok &= ok;
            } catch (const std::exception& ex) {
                std::cerr << "client failed: " << ex.what() << "\n";
                all_ok = false;
            }
        }

        // stop server by waking it with a local connection
        http.stop();
        try { http_request(port); } catch (...) {}

        if (server_thread.joinable())
            server_thread.join();

        std::cout << "Server test " << (all_ok ? "PASSED" : "FAILED") << "\n";
        return all_ok ? 0 : 1;

    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << "\n";
        return 1;
    }
}
