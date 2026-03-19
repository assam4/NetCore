#include "server_types.hpp"
#include "virtualhost.hpp"
#include <iostream>
#include <cstdlib>
#include <exception>
#include <cctype>

namespace   http {
    namespace   core {
        namespace   types {

            __listen&    __listen::fill(std::string field, const std::vector<VirtualHost>& servers) {
                if (field.empty())
                    return *this;
                else if (field[0] == '[')  set_ipv6(field);
                else if (field.find_first_of('.') != std::string::npos) set_ipv4(field);
                if (!field.empty()) {
                    if (field[0] == ':')
                        field.erase(0, 1);
                    set_port(field);
                }
                if (!field.empty()) throw std::runtime_error("Incorrect remaining part of a line: '" + field + " '.");
                default_server = true;
                for (std::vector<VirtualHost>::const_iterator it = servers.begin(); it != servers.end(); ++it)
                    if (it->get_listen().find(*this) != it->get_listen().end())
                        default_server = false;
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
       
            
            void    __serv_name::fill_server_names(std::set<std::string>& data, const std::vector<http::core::VirtualHost>& servers) {
                (void)servers;
                if (data.empty())
                    server_name.push_back("");
                else
                    server_name.assign(data.begin(), data.end());
            }


            void    __content::fill_error_pages(const std::map<std::set<std::string>, std::string>& data) {
                for ( std::map<std::set<std::string>, std::string>::const_iterator mit = data.begin(); mit != data.end(); ++mit) {
                    if (mit->second.empty() || mit->second[0] != '/')
                        throw std::runtime_error("Invalid error_pages path: " + mit->second);
                     for (std::set<std::string>::const_iterator it = mit->first.begin(); it != mit->first.end(); ++it) {
                        if ((*it).empty() || (*it).length() != 3)
                            throw std::runtime_error("Invalid err_pages: " + (*it));
                        int err = std::atoi((*it).c_str());
                        if (err < 300 || err > 599)
                            throw std::runtime_error("Invalid err_pages: " + (*it));
                        error_pages[static_cast<uint16_t>(err)] = mit->second;
                     }
                }
            }
        
            void    __content::fill_index(const std::set<std::string>& data) {
                if (data.empty())
                    index.insert("index.html");
                else
                    index = data;
            }
         
            void    __content::fill_allowed_methods(const std::set<std::string>& data) {
                if (data.empty())
                    return;
                else
                    allowed_methods = data;
            }

            void    __content::fill_root(const std::string& data) {
                if (data.empty())
                    root = "html";
                else
                    root = data;
             }

            void    __content::fill_max_body_size(const std::string& data) {
                if (data.empty()) {
                    client_max_body_size = 1024 * 1024;
                    return ;
                }
                size_t  end = data.find_first_not_of("0123456789");
                if (end != std::string::npos && end != data.length() - 1)
                    throw std::runtime_error("Invalid client_max_body field: " + data);
                size_t  num = std::atoi(data.c_str());
                if (end != std::string::npos)
                    switch(data[end]) {
                        case 'g':   num = num * 1024 * 1024 * 1024; break;
                        case 'm':   num = num * 1024 * 1024;        break ;
                        case 'k':   num = num * 1024;               break ;
                        default:    throw std::runtime_error("Invalid client_max_body field: " + data);
                    }
                client_max_body_size = num;
            }
                
            void    __content::fill_autoindex(const std::string& data) {
                    if (data.empty() || data == "off")
                        autoindex = false;
                    else if (data == "on")
                        autoindex = true;
                    else
                        throw std::runtime_error("Invalid autoindex field: " + data);
            }


            void    __route::fill_redirection(const std::pair<std::string, std::string>& data) {
                if (data.first.empty() || data.second.empty()) {
                    code = 0;
                    return ;
                }
                int num = std::atoi(data.first.c_str());
                if (data.first.length() != 3 || !(num > 200 && num < 600))
                    throw std::runtime_error("Ivalid Redirection code: " + data.first);
                else {
                    code = num;
                    new_path = data.second;
                }
            }
        }
    }
}