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
            /**
            * @struct __listen
            * @brief Represents server listen configuration (address and port binding).
            * @details Encapsulates host address and port information with validation.
            *          Supports IPv4, IPv6, and domain names with default server flag.
            *          Supports comparison operators for storage in containers.
            */
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
                    __listen&    fill(std::string, const std::vector<VirtualHost>&);
                private:
                    void    set_ipv6(std::string&);
                    void    set_ipv4(std::string&);
                    void    set_port(std::string&);
            };

            /**
            * @struct __serv_name
            * @brief Holds server name (domain) aliases for virtual host matching.
            * @details Manages multiple domain names that this virtual host responds to.
            *          Supports wildcard domain matching and FQDN patterns.
            */
            struct  __serv_name {
                std::vector<std::string>    server_name;

                void  fill_server_names(std::set<std::string>&, const std::vector<VirtualHost>&);
            };

            /**
            * @struct __content
            * @brief Contains all HTTP response configuration and content delivery settings.
            * @details Aggregates error pages, index files, request methods, root directory,
            *          body size limits, and directory listing options. Used by both server
            *          and location blocks for inheritance and overriding.
            */
            struct  __content {
                std::map<uint16_t, std::string> error_pages;
                std::set<std::string>   index;
                bool   allowed_methods[3];
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

            /**
            * @struct __route
            * @brief Represents a location routing rule with optional redirects.
            * @details Stores location path, matching modifier (exact, prefix, regex),
            *          and optional redirect information (code and target path).
            *          Enables flexible request routing within server blocks.
            */
            struct  __route {
                std::string path;
                std::string modifier;  // need to validate
                uint16_t    code;
                std::string new_path;

                __route(): code(0) {}

                void    fill_redirection(const std::pair<std::string, std::string>&);
                void    fill_location_path(const std::string&);
                void    fill_location_modifier(const std::string&);
            };

            /**
            * @struct __location
            * @brief Complete location block configuration combining routing and content rules.
            * @details Combines route matching rules (path, modifier, redirects) with content
            *          delivery settings (root, autoindex, methods). Includes CGI and upload
            *          support for dynamic request handling.
            */
            struct  __location {
                __route                 route;
                __content               content;
                std::set<std::string>   cgi_extension;
                std::string             upload_location;

                void    fill_cgi_extension(const std::set<std::string>&);
                void    fill_upload_location(const std::string&);
            };

        }
    }
}

#endif
