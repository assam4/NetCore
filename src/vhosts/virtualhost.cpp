#include <sstream>
#include <stdexcept>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include "virtualhost.hpp"
#include "utils.hpp"

namespace http {
    namespace core {

        VirtualHost VirtualHost::build(const config::parser::__server_row_data& data, const std::vector<VirtualHost>& servers) {
            return VirtualHost()
                    .set_listen(data.listen, servers)
                    .set_server_names(data.server_name)
                    .set_error_pages(data.error_pages)
                    .set_index(data.index)
                    .set_allowed_methods(data.allowed_methods)
                    .set_root(data.root)
                    .set_client_max_body_size(data.client_max_body_size)
                    .set_autoindex(data.autoindex)
                    .set_return(data.ret_redirection)
                    .set_locations(data.locations);
        }

        VirtualHost& VirtualHost::set_listen(const std::set<std::string>& data, const std::vector<VirtualHost>& servers) {
            for (std::set<std::string>::const_iterator it = data.begin(); it != data.end(); ++it) {
                if (it->compare(0, 9, "localhost") == 0) {
                    listen.insert(types::__listen().fill("127.0.0.1" + (*it).substr(9), servers));
                    listen.insert(types::__listen().fill("[::1]" + (*it).substr(9), servers));

                }
                else
                    listen.insert(types::__listen().fill(*it, servers));
            }
            return *this;
        }

        VirtualHost& VirtualHost::set_server_names(const std::set<std::string>& data) {
            std::set<std::string> mutable_names = data;
            server_name.fill_server_names(mutable_names);
            return *this;
        }

        VirtualHost& VirtualHost::set_error_pages(const std::map<std::set<std::string>, std::string>& data) {
            content.fill_error_pages(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_index(const std::set<std::string>& data) {
            content.fill_index(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_allowed_methods(const std::set<std::string>& data) {
            content.fill_allowed_methods(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_root(const std::string& data) {
            content.fill_root(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_client_max_body_size(const std::string& data) {
            content.fill_max_body_size(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_autoindex(const std::string& data) {
            content.fill_autoindex(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_return(const std::pair<std::string, std::string>& data) {
            route.fill_redirection(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_location_path(const std::string& data) {
            route.fill_location_path(data);
            return *this;
        }

        VirtualHost& VirtualHost::set_location_modifier(const std::string& data) {
            route.fill_location_modifier(data);
            return *this;
        }

        types::__location VirtualHost::create_default_location() {
            types::__location loc;
            loc.route.path = "/";
            loc.content.root = this->content.root;
            loc.content.index = this->content.index;
            loc.content.allowed_methods = this->content.allowed_methods;
            loc.content.autoindex = this->content.autoindex;
            loc.content.error_pages = this->content.error_pages;
            loc.content.client_max_body_size = this->content.client_max_body_size;
            return loc;
        }

        VirtualHost& VirtualHost::set_locations(const std::vector<config::parser::__location_row_data>& data) {
            locations.clear();
            bool has_root_location = false;
            for (std::vector<config::parser::__location_row_data>::const_iterator it = data.begin();
                 it != data.end();
                 ++it) {
                types::__location loc;
                loc.route.fill_location_path(it->path);
                loc.route.fill_location_modifier(it->modifier);
                loc.route.fill_redirection(it->ret_redirection);

                loc.content.fill_error_pages(it->error_pages);
                loc.content.fill_index(it->index);
                loc.content.fill_allowed_methods(it->allowed_methods);
                loc.content.fill_root(it->root);
                loc.content.fill_max_body_size(it->client_max_body_size);
                loc.content.fill_autoindex(it->autoindex);

                loc.fill_cgi_extension(it->cgi_extension);
                loc.fill_upload_location(it->upload_location);
                locations.push_back(loc);
                if (loc.route.path == "/")
                    has_root_location = true;
            }
            if (!has_root_location)
                locations.push_back(create_default_location());
            return *this;
        }

        std::vector<std::pair<sockaddr_storage, socklen_t> >    transform_to_sstorage(const VirtualHost& vh) {
            std::vector<std::pair<sockaddr_storage, socklen_t> > result;
            for (std::set<types::__listen>::const_iterator it = vh.get_listen().begin(); it != vh.get_listen().end(); ++it) {
                addrinfo hints;
                std::memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_PASSIVE;
                std::ostringstream  port_ss;
                port_ss << it->port;
                const std::string   port = port_ss.str();
                addrinfo* res = NULL;
                if (getaddrinfo(it->host.c_str(), port.c_str(), &hints, &res) != 0 || !res)
                    throw std::runtime_error("getaddrinfo failed");
                sockaddr_storage    storage;
                std::memset(&storage, 0, sizeof(storage));
                if (res->ai_addrlen > sizeof(storage)) {
                    freeaddrinfo(res);
                    throw std::runtime_error("address too large for sockaddr_storage");
                }
                std::memcpy(&storage, res->ai_addr, res->ai_addrlen);
                socklen_t len = static_cast<socklen_t>(res->ai_addrlen);
                freeaddrinfo(res);
                result.push_back(std::make_pair(storage, len));
            }
            return result;
        }

        std::string sockaddr_to_string(sockaddr_storage& ss) {
            char ip[INET6_ADDRSTRLEN];
            if (ss.ss_family == AF_INET) {
                sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&ss);
                inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                return std::string(ip) + ":" + to_string(ntohs(addr->sin_port));
            } else if (ss.ss_family == AF_INET6) {
                sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(&ss);
                inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
                return std::string(ip) + ":" + to_string(ntohs(addr->sin6_port));
            }
            return "unknown";
        }

    }
}
