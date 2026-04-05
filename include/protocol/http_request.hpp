#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <stdint.h>
# include <map>
# include <vector>
# include <string>
# include <exception>
# include "server_types.hpp"
# include "http_types.hpp"

namespace http {
	namespace core {

        class Request {
            public:
                struct  __http_request {
                    std::map<std::string, std::vector<std::string> > headers;
                    std::string query;
                    std::string body;
                    std::string version;
                    std::string uri;
                    types::HttpMethod method;
                };
                static  std::pair<types::HttpStatus, __http_request> parse_message(const std::string&, size_t max_body_size=1024);
            private:
                Request();
                static bool    is_query_char(unsigned char c);
                static void    validate_query_string(const std::string& query);
                static void    validate_uri(Request::__http_request& hr);
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
