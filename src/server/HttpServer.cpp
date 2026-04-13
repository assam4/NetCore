#include <algorithm>
#include "HttpServer.hpp"
#include "config_store.hpp"
#include "utils.hpp"

#if defined(__linux__)
# include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
# include <sys/event.h>
# else
	#error "Unsupported platform!"
#endif

namespace http {
	namespace core {

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
					std::cout << "Listener -> " << sockaddr_to_string(addrs[j].first) << std::endl;
					_server.add_socket(ss);
					AcceptHandler* ah = new AcceptHandler(raw_fd, _server, _dispatcher, *this);
					#if defined(__linux__)
						_dispatcher.register_handler(ah, EPOLLIN);
					#elif defined(__APPLE__) || defined(__FreeBSD__)
						_dispatcher.register_handler(ah, EVFILT_READ);
					#endif
				}
			}
			if (_server.get_sockets().size() == 0)
				throw std::runtime_error("Not have listener");
		}

		const VirtualHost* HttpServer::find_vhost(uint16_t port, const std::string& host_head) {
			const VirtualHost* default_vhost = NULL;
			std::string host = host_head;
			size_t colon_pos = host.find(':');
			if (colon_pos != std::string::npos)
				host.erase(colon_pos);
			std::transform(host.begin(), host.end(), host.begin(), static_cast<int(*)(int)>(std::tolower));
			for (size_t i = 0; i < _virtual_hosts.size(); ++i) {
				const std::set<types::__listen>& listens = _virtual_hosts[i].get_listen();
				for (std::set<types::__listen>::const_iterator it = listens.begin(); it != listens.end(); ++it) {
					if (port != it->port)
						continue;
					const std::vector<std::string>& domain_names = _virtual_hosts[i].get_server_name();
					for (size_t n = 0; n < domain_names.size(); ++n) {
						if (host == domain_names[n])
							return &_virtual_hosts[i];
					}
					if (it->default_server || default_vhost == NULL)
						default_vhost = &_virtual_hosts[i];
				}
			}
			return default_vhost;
		}

		void HttpServer::run() {
			_dispatcher.run();
		}

		void HttpServer::stop() { _dispatcher.stop(); }

	}
}
