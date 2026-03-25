#include "HttpServer.hpp"
#include <csignal>
#include "config_store.hpp"

#if defined(__linux__)
# include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
# include <sys/event.h>
# else
	#error "Unsupported platform!"
#endif

namespace http {
	namespace core {

		volatile sig_atomic_t HttpServer::_shutdown = 0;

		void HttpServer::signal_handler(int sig) {
			static_cast<void>(sig);
			_shutdown = 1;
		}

		HttpServer::HttpServer() {}

		HttpServer::~HttpServer() {}

		void HttpServer::init(const std::vector<http::config::parser::__server_row_data>& row_config) {
			_virtual_hosts = ConfigStore::collect(row_config);
			std::set<types::__listen> seen;

			for (size_t i = 0; i < _virtual_hosts.size(); ++i) {
				std::vector<std::pair<sockaddr_storage, socklen_t> > addrs = transform_to_sstorage(_virtual_hosts[i]);
				const std::set<types::__listen>& listens = _virtual_hosts[i].get_listen();
				std::set<types::__listen>::const_iterator lit = listens.begin();
				 for (size_t j = 0; j < addrs.size() && lit != listens.end(); ++j, ++lit) {
					if (seen.count(*lit))
						continue ;
					seen.insert(*lit);
					int raw_fd = ::socket(addrs[j].first.ss_family, SOCK_STREAM, 0);
					if (raw_fd < 0) {
						std::cerr << "socket() failed";
						continue ;
					}
					ServerSocket* ss = new ServerSocket(raw_fd);
					if (::bind(raw_fd, reinterpret_cast<sockaddr*>(&addrs[j].first), addrs[j].second) < 0) {
						delete ss;
						std::cerr << "bind() failed for " << lit->host << ":" << lit->port << "\n";
						continue ;
					}
					ss->listen();
					_server.add_socket(ss);
					AcceptHandler* ah = new AcceptHandler(raw_fd, _server, _dispatcher);
					#if defined(__linux__)
						_dispatcher.register_handler(ah, EPOLLIN);
					#elif defined(__APPLE__) || defined(FreeBSD__)
						_dispatcher.register_handler(ah, EVFILT_READ);
					#endif
				}
			}
		}

		const VirtualHost* HttpServer::match_vhost(uint16_t port, const std::string& host_head) {
			const VirtualHost* default_vhost = NULL;
			for (size_t i = 0; i < _virtual_hosts.size(); ++i) {
				const std::set<types::__listen>& listens = _virtual_hosts[i].get_listen();
				for (std::set<types::__listen>::const_iterator it = listens.begin(); it != listens.end(); ++it) {
					if (port != it->port)
						continue;
					const std::vector<std::string>& domain_names = _virtual_hosts[i].get_server_name();
					for (size_t n = 0; n < domain_names.size(); ++n) {
						if (host_head == domain_names[n])
							return &_virtual_hosts[i];
					}
					if (it->default_server || default_vhost == NULL)
						default_vhost = &_virtual_hosts[i];
				}
			}
			return default_vhost;
		}

		void HttpServer::run() {
			signal(SIGINT, HttpServer::signal_handler);
			signal(SIGTERM, HttpServer::signal_handler);
			signal(SIGPIPE, SIG_IGN);
			_dispatcher.run(_shutdown);
		}

		void HttpServer::stop() { _dispatcher.stop(); }

	}
}
