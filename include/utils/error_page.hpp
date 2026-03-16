#ifndef ERROR_PAGE_HPP
# define ERROR_PAGE_HPP

# include <map>
# include <string>
# include <fstream>
# include <sstream>

namespace http {
    namespace core {
        namespace page_generator {

            class ErrorPageGenerator {
            public:
                static std::string generate_page(int status_code);

            private:
                static std::map<int, std::string> http_errors;
                static void fill_errors();
            };

        }
    }
}

#endif