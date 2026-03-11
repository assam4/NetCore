#ifndef SERVER_TYPES_HPP
# define SERVER_TYPES_HPP

# include <map>
# include <vector>
# include <set>
# include <string>
# include <stdint.h>

namespace   http {
    namespace core {
        
        class   VirtualHost;
        
        namespace types {
            
            struct  __listen {
                    std::string host;
                    uint16_t    port;
                    bool        default_server; //  last become a default
                    /* for future project expansion
                    bool    multi_user_port; 
                    bool    ipv6_only;
                    bool    ssl; */

                    __listen(): host("0.0.0.0"), port(80), default_server(false) {}
                    bool    operator<(const __listen& other) const throw() { return (host == other.host) ? (port < other.port) : (host < other.host); }
                    bool    operator==(const __listen& other) const throw() { return host == other.host && port == other.port; }
                    __listen&    fill(std::string);
                private:
                    void    set_ipv6(std::string&);
                    void    set_ipv4(std::string&);
                    void    set_port(std::string&);
            };
        
            struct  __serv_name {
                std::vector<std::string>    server_name;

                void  fill_server_names(std::set<std::string>&, const std::vector<http::core::VirtualHost>&);
            };

            struct  __content {
                std::map<uint16_t, std::string> error_pages;
                std::set<std::string>   index;
                std::set<std::string>   allowed_methods;
                std::string root;
                size_t  client_max_body_size;
                bool autoindex;

                void    fill_error_pages(const std::map<std::set<std::string>, std::string>&);
                void    fill_index(const std::set<std::string>&);
                void    fill_allowed_methods(const std::set<std::string>&);
                void    fill_root(const std::string&);
                void    fill_max_body_size(const std::string&);
                void    fill_autoindex(const std::string&);
            };
        
            struct  __route {
                std::string path;
                std::string modifier;
                uint16_t    code;
                std::string new_path;

                __route(): code(0) {}

                void    fill_redirection(const std::pair<std::string, std::string>&);
                void    fill_location_path(const std::string& data) { path = data; }
                void    fill_location_modifier(const std::string& data) { modifier = data; }
            };

            struct  __location {
                __route                 route;
                __content               content;
                std::set<std::string>   cgi_extension;
                std::string             upload_location;

                void    fill_cgi_extension(const std::set<std::string>& data)  { cgi_extension = data; }
                void    fill_upload_location(const std::string& data)           { upload_location = data; }
            };

        }
    }
}

#endif
