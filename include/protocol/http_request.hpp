#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <stdint.h>
# include <map>
# include <vector>
# include <string>
# include <exception>
# include <sstream>
# include <cctype>
# include <sys/socket.h>
# include <unistd.h>
# include "server_types.hpp"
# include "http_types.hpp"
# include "Server.hpp"
# include "utils.hpp"

namespace http {
	namespace core {

		// HTTP Request parsing constants
		const static size_t StartLineLength = 8192;      // 8KB
		const static size_t URILength = 4096;            // 4KB
		const static size_t QueryLength = 2048;          // 2KB
		const static size_t SingleHeaderLength = 8192;   // 8KB
		const static size_t HeadersLength = 32768;       // 32KB
		const static size_t DefaultMaxBodyLength = 1048576; // 1MB

		/**
		 * @brief Parsed HTTP request start line.
		 * Stores method, normalized URI, optional query string,
		 * and validated HTTP protocol version.
		 */
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

				std::string uri_decode(const std::string& str) {
					std::string decoded;
					for (size_t i = 0; i < str.size(); ++i) {
						if (str[i] == '%' && i + 2 < str.size() && std::isxdigit(str[i + 1]) && std::isxdigit(str[i + 2])) {
							unsigned char high = std::isdigit(str[i + 1]) ? str[i + 1] - '0' : std::tolower(str[i + 1]) - 'a' + 10;
							unsigned char low = std::isdigit(str[i + 2]) ? str[i + 2] - '0' : std::tolower(str[i + 2]) - 'a' + 10;
							decoded += static_cast<char>((high << 4) | low);
							i += 2;
						} else
							decoded += str[i];
					}
					return decoded;
				}

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
					uri = uri_decode(uri);
					if (uri.find('\0') != std::string::npos)
						throw types::BAD_REQUEST;
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

		/**
		 * @brief Normalized HTTP header storage and validation helper.
		 * Parses header lines into a lowercase key/value map,
		 * while enforcing uniqueness constraints for selected headers.
		 */
		struct __header_map {
				const static std::set<std::string> uniques;
				std::map<std::string, std::vector<std::string> > header_map;

				static std::set<std::string> make_unique_headers_list();
				void    parse_header(const std::string&, size_t&);
			private:
				std::vector<std::string> parse_values(const std::string&) const;
		};

		/**
		 * @brief HTTP request body reader.
		 * Supports fixed-size payloads and chunked transfer decoding
		 * using the shared connection buffer APIs.
		 */
		struct __body {
				std::string content;

				void    fixed_read(Connection&, size_t);
				void    chunked_read(Connection&, const std::string&);

			private:
				size_t  append_to(Connection&, std::string&, const std::string&, size_t);
				void	try_to_read(Connection&) const;
				size_t  get_chunk_size(Connection&);
				void	check_end_of(Connection&) const;
		};

		/**
		 * @brief Parsed HTTP request aggregate.
		 * Combines start-line, headers, and body, and exposes
		 * a static parse entry point for request construction.
		 */
		class Request {
			public:
				__start_line    start_line;
				__header_map    headers;
				__body          body;
				static  std::pair<types::HttpStatus, Request> parse_message(Connection&);
			private:
				Request();
				void    parse_start_line(const std::string&);
				std::map<std::string, std::vector<std::string> >::const_iterator    check_mandatory_headers() const;
		};
	}
}

#endif
