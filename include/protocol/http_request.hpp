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

        struct __start_line {
                std::string         uri;
                std::string         query;
                std::string         version;
                types::HttpMethod   method;
                
                void    critical_cases_check(const std::string& line) const {
                    if (line.empty() || line.length() > StartLineLength 
					        || line.find_first_of("\r\n\t") != std::string::npos)
				    throw types::BAD_REQUEST;
                }

                void    parse_method(const std::string& line, size_t start, size_t end) {
                    if (line.compare(start, end, "GET") == 0)			method = types::GET;
			        else if (line.compare(start, end, "POST") == 0)		method = types::POST;
			        else if (line.compare(start, end, "DELETE") == 0)	method = types::DEL;
			        else
                        throw types::NOT_IMPLEMENTED;
                }

                void    parse_uri(const std::string& line, size_t start, size_t end) {
                    size_t  length = end - start;
		            if (!length)
                        throw types::BAD_REQUEST;
			        uri = line.substr(start, length);
			        extract_uri();
                }
            
                void    parse_protocol(const std::string&, size_t, size_t);
            private:
                void    extract_uri() {
                    if (uri[0] == '/')
				        extract_origin();
			        else if (uri.compare(0, 7, "http://") == 0)
				        extract_absolute();
			        else
				        throw types::BAD_REQUEST;
                    query_validate();
                }

                void    extract_origin() {
                    size_t  query_start_pos = uri.find_first_of('?');
                    if (query_start_pos != std::string::npos) {
                        query = uri.substr(query_start_pos + 1);
                        uri.erase(query_start_pos);
                    }
                    if (uri.length() > URILength)
                        throw types::URI_TOO_LONG;
                }

                void    extract_absolute() {
                    size_t  uri_start_pos = uri.find_first_of('/', 8);
                    if (uri_start_pos == std::string::npos)
                        throw types::BAD_REQUEST;
                    uri.erase(0, uri_start_pos);
                    extract_origin();
                }

                void    query_validate() const;
            };

        struct __header_map {
                const static std::set<std::string> uniques;
                std::map<std::string, std::vector<std::string> > header_map;

                static std::set<std::string>   make_unique_headers_list();
                void    parse_header(const std::string&, size_t&);
            private:
                std::vector<std::string> parse_values(const std::string&) const;
        };

        class Request {
            public:
                __start_line    start_line;
                __header_map    headers;

                static  std::pair<types::HttpStatus, Request> parse_message(const std::string&);
            private:
                Request();
                void    parse_start_line(const std::string&);
                void    check_mandatory_headers() const;
        };
    }
}

#endif
