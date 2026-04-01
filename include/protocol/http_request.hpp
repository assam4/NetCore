#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <stdint.h>
# include <map>
# include <vector>
# include <string>
# include <exception>
# include "server_types.hpp"

namespace http {
    namespace core {

        class Request {
            public:
                struct  __http_request {
                    std::map<std::string, std::vector<std::string> > headers;
                    std::map<std::string, std::string> querry;
                    std::string body;
                    std::string version;
                    std::string uri;
                    uint8_t method; 
                };
                static  std::pair<uint16_t, __http_request> parse_message(const std::string&, size_t);
            private:
                Request();
                static void    parse_method(__http_request&, const std::string&, size_t&);
                static void    parse_uri(__http_request&, const std::string&, size_t&);
                static void    parse_protocol(__http_request&, const std::string&, size_t&);
                static void    parse_start_line(std::stringstream&, __http_request&);
                static void    parse_headers(std::stringstream&, __http_request&);
                static void    parse_body(std::stringstream&, __http_request&, size_t);
        };


    }
}

#endif