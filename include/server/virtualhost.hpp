#ifndef VIRTUAL_HOST_HPP
# define VIRTUAL_HOST_HPP

# include <map>
# include <set>
# include <string>
# include <vector>
# include <utility>
# include "server_types.hpp"
# include "configparser.hpp"

namespace http {
    namespace core {

        class VirtualHost {
            public:
                static VirtualHost build(const config::parser::__server_row_data&, const std::vector<VirtualHost>&);
                VirtualHost& set_listen(const std::set<std::string>& data);
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

                const std::string& get_host() const { return listen.host; }
                uint16_t get_port() const { return listen.port; }
                bool is_default_server() const { return listen.default_server; }
                const std::vector<std::string>& get_server_name() const { return server_name.server_name; }
                const std::map<uint16_t, std::string>& get_error_pages() const { return content.error_pages; }
                const std::set<std::string>& get_index() const { return content.index; }
                const std::set<std::string>& get_allowed_methods() const { return content.allowed_methods; }
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

                types::__listen listen;
                types::__serv_name server_name;
                types::__content content;
                types::__route route;
                std::vector<types::__location> locations;
        };
    }
}

#endif
