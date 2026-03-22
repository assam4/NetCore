#include "HttpServer.hpp"
#include <sys/epoll.h>
#include <csignal>
#include "config_store.hpp"

volatile sig_atomic_t HttpServer::_shutdown = 0;

void HttpServer::signal_handler(int sig) {
	static_cast<void>(sig);
	_shutdown = 1;
}

HttpServer::HttpServer() {}

HttpServer::~HttpServer() {}

void HttpServer::init(const std::vector<http::config::parser::__server_row_data>& row_config) {
	_virtual_hosts = http::core::ConfigStore::collect(row_config);
	// add this temporarily right after ConfigStore::collect()
	const std::vector<http::core::types::__location>& locs = _virtual_hosts[0].get_locations();
	std::cout << "\n--- actual location paths ---\n";
	for (size_t i = 0; i < locs.size(); ++i) {
	    std::cout << "  path='" << locs[i].route.path << "'"
	              << " modifier='" << locs[i].route.modifier << "'\n";
	}
	std::set<http::core::types::__listen> seen;

	for (size_t i = 0; i < _virtual_hosts.size(); ++i) {
		std::vector<std::pair<sockaddr_storage, socklen_t> > addrs = http::core::transform_to_sstorage(_virtual_hosts[i]);
		const std::set<http::core::types::__listen>& listens = _virtual_hosts[i].get_listen();
		std::set<http::core::types::__listen>::const_iterator lit = listens.begin();
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
			_dispatcher.register_handler(ah, EPOLLIN);
		}
	}
}

const http::core::VirtualHost* HttpServer::match_vhost(uint16_t port, const std::string& host_head) {
	const http::core::VirtualHost* default_vhost = NULL;
	for (size_t i = 0; i < _virtual_hosts.size(); ++i) {
		const std::set<http::core::types::__listen>& listens = _virtual_hosts[i].get_listen();
		for (std::set<http::core::types::__listen>::const_iterator it = listens.begin(); it != listens.end(); ++it) {
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
