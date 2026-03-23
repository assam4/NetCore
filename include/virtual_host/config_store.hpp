#ifndef CONFIG_STORE_HPP
# define CONFIG_STORE_HPP

# include  "virtualhost.hpp"

namespace http {
    namespace core {
        class VirtualHost;
        
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