#ifndef CONFIG_STORE_HPP
# define CONFIG_STORE_HPP

# include  "virtualhost.hpp"

namespace http {
    namespace core {
        /**
        * @class VirtualHost
        * @brief Forward declaration of runtime virtual-host model.
        * @details Used to avoid heavy include dependencies in store declarations.
        *          Full class definition is available in virtualhost.hpp.
        */
        class VirtualHost;

        /**
        * @class ConfigStore
        * @brief Converts parsed configuration rows into validated virtual hosts.
        * @details Builds hosts sequentially so each new host can validate conflicts
        *          against previously constructed entries (listen/name uniqueness).
        */
        class  ConfigStore {
            public:
                static  std::vector<VirtualHost>    collect(const std::vector<config::parser::__server_row_data>& data) {
                    std::vector<VirtualHost>    result;
                    for (std::vector<config::parser::__server_row_data>::const_iterator it = data.begin(); it != data.end(); ++it)
                        result.push_back(VirtualHost::build(*it, result));
                    return result;
                }
            private:
                ConfigStore();
        };
    }
}



#endif