#ifndef SERVER_CONFIG_STORE_HPP
# define SERVER_CONFIG_STORE_HPP

# include <map>
# include <vector>
# include <set>
# include <string>
# include <stdint.h>
# include <arpa/inet.h>

namespace   http {
    namespace   core {

        struct  __listen {
            std::string host;
            uint16_t    port;
            bool    default_server; //  last become a default
            // for future project expansion
            bool    multi_user_port; 
            bool    ipv6_only;
            bool    ssl;

            __listen() : host("0.0.0.0"), port(80), default_server(false), multi_user_port(false), ipv6_only(false), ssl(false){
            }

            __listen&    fill(std::string);
            
            bool    operator<(const __listen& other) const {
                if (host == other.host)
                    return port < other.port;
                else
                    return host < other.host;
            }
            
            bool    operator==(const __listen& other) const {
                return host == other.host && port == other.port;
            }
            
            private:
            void    set_ipv6(std::string&);
            void    set_ipv4(std::string&);
            void    set_port(std::string&);
        };

        struct  __server_paths {
            std::map<uint16_t, std::string>   error_pages;

            bool    validate_path(const std::string& path) {
                if (path.empty() || path.front() != '/')
                    
            }

            void    fill_err_pages(const std::map<std::set<std::string>, std::string>& data) {
                for ( std::map<std::set<std::string>, std::string>::const_iterator mit = data.begin(); mit != data.end(); ++mit) {
                    if (!validate_path(mit->second))
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
        };

        class   ServerConfigStore {
            public:
                static  ServerConfigStore   clone() {
                    return ServerConfigStore();
                }
                ServerConfigStore&  set_listen(const std::set<std::string>&);
                ServerConfigStore&  set_server_name(const std::set<std::string>&, const std::vector<ServerConfigStore>&);
            private:
                ServerConfigStore();


      
                std::set<__listen>          listen;
                std::vector<std::string>    server_name;
            

        };
    }
}


#endif