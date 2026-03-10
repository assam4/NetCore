#include "server_config_store.hpp"
#include <iostream>
#include <cstdlib>
#include <exception>
#include <cctype>



namespace   http {
    namespace core {

            __listen&    __listen::fill(std::string field) {
                if (field.empty())  throw std::runtime_error("Empty listen field!.");
                else if (field.front() == '[')  set_ipv6(field);
                else if (field.find_first_of('.') != std::string::npos) set_ipv4(field);
                if (!field.empty()) {
                    if (field.front() == ':')
                        field.erase(0, 1);
                    set_port(field);
                }
                if (!field.empty()) throw std::runtime_error("Incorrect remaining part of a line: '" + field + " '.");
                return *this;
            }

            void    __listen::set_ipv6(std::string& field) {
                size_t  end = field.find_first_of(']', 1);
                if (end == std::string::npos)
                        throw std::runtime_error("Ivalid listen field: '" + field + " '.");
                    host = field.substr(1, end - 1);
                    field.erase(0, end + 1);
            }

            void    __listen::set_ipv4(std::string& field) {
                size_t  end = field.find_first_of(':');
                if (end == std::string::npos) {
                    host = field;
                    field.clear();
                }
                else {
                    host = field.substr(0, end);
                    field.erase(0, end);
                }
            }

            void    __listen::set_port(std::string& field) {
                size_t end = 0;
                for(; end < field.length() && std::isdigit(static_cast<unsigned char>(field[end])); ++end) ;
                int port = std::atoi(field.c_str());
                if (port < 1 || port > 65535 || end > 5)
                    throw std::runtime_error("Port must be in in range 1-65535");
                this->port = port;
                field.erase(0, end);
            }  
             
            
            ServerConfigStore&  ServerConfigStore::set_listen(const std::set<std::string>& data) {
                for (std::set<std::string>::const_iterator it = data.begin(); it != data.end(); ++it) {
                    std::pair<std::set<__listen>::iterator, bool> result = listen.insert(__listen().fill(*it));
                if (!result.second)
                    throw std::runtime_error("ip/port already used: " + *it);
                }
                return *this;
            }
            
            static bool    has_intersection(const std::set<__listen>& first, const std::set<__listen>& second) {
                for (std::set<__listen>::const_iterator it = first.begin(); it != first.end(); ++it)
                    if (second.find(*it) != second.end())
                        return true;
                return false;
            }

            ServerConfigStore&  ServerConfigStore::set_server_name(std::set<std::string>& data, const std::vector<ServerConfigStore>& servers) {
                for (std::vector<ServerConfigStore>::const_iterator cit = servers.begin(); cit != servers.end(); ++cit) {
                    if (!has_intersection(this->listen, cit->listen))
                        continue ;
                    for (std::set<std::string>::iterator it = data.begin(); it != data.end();) {
                        if (cit->server_name.find(*it) != cit->server_name.end()) {
                            std::cerr << "Warning: ignoring server-name: " << *it << std::endl;
                            data.erase(it++);
                        }
                        else
                            ++it;
                    }
                }
                server_name = data;
                return *this;
            }
}