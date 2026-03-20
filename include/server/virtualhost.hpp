#ifndef VIRTUAL_HOST_HPP
# define VIRTUAL_HOST_HPP

# include <map>
# include <set>
# include <string>
# include <vector>
# include <utility>
# include <sys/socket.h>
# include "server_types.hpp"
# include "configparser.hpp"   

namespace http {
    namespace core {

        /**
        * @class VirtualHost
        * @brief Runtime representation of a fully parsed and validated nginx-style virtual host configuration.
        * @details Provides type-safe access to server listen parameters, names, content delivery rules,
        *          location-specific routing, and request handling options. Built from raw parser output
        *          using builder pattern with full validation. Immutable after construction for thread safety.
        */
        class VirtualHost {
            public:
                static VirtualHost build(const config::parser::__server_row_data&, const std::vector<VirtualHost>&);
                VirtualHost& set_listen(const std::set<std::string>& data, const std::vector<VirtualHost>&);
                VirtualHost& set_server_names(const std::set<std::string>& data, const std::vector<VirtualHost>& servers);
                VirtualHost& set_error_pages(const std::map<std::set<std::string>, std::string>& data);
                VirtualHost& set_index(const std::set<std::string>& data);
                VirtualHost& set_allowed_methods(const std::set<std::string>& data);
                VirtualHost& set_root(const std::string& data);
                VirtualHost& set_client_max_body_size(const std::string& data);
                VirtualHost& set_autoindex(const std::string& data);
                VirtualHost& set_return(const std::pair<std::string, std::string>& data);
                VirtualHost& set_location_path(const std::string& data);
                VirtualHost& set_location_modifier(const std::string& data);
                VirtualHost& set_locations(const std::vector<config::parser::__location_row_data>& data);

                const std::set<types::__listen> get_listen() const { return listen; }
                const std::vector<std::string>& get_server_name() const { return server_name.server_name; }
                const std::map<uint16_t, std::string>& get_error_pages() const { return content.error_pages; }
                const std::set<std::string>& get_index() const { return content.index; }
                const bool* get_allowed_methods() const { return content.allowed_methods; }
                const std::string& get_root() const { return content.root; }
                size_t get_max_body_size() const { return content.client_max_body_size; }
                bool get_autoindex() const { return content.autoindex; }
                const std::string& get_path() const { return route.path; }
                const std::string& get_modifier() const { return route.modifier; }
                uint16_t get_redirect_code() const { return route.code; }
                const std::string& get_redirect_path() const { return route.new_path; }
                const std::vector<types::__location>& get_locations() const { return locations; }

            private:
                VirtualHost() {}

                std::set<types::__listen> listen;
                types::__serv_name server_name;
                types::__content content;
                types::__route route;
                std::vector<types::__location> locations;
        };

        std::vector<std::pair<sockaddr_storage, socklen_t> >    transform_to_sstorage(const VirtualHost&);
    }
}

#endif
