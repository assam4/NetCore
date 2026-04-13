#include "http_request.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <stdexcept>

namespace http {
	namespace core {

		static const size_t MaxAttempts = 10000;

		static void read_with_attempts(Connection& c, types::HttpStatus on_timeout) {
			size_t attempts = 0;
			while (attempts < MaxAttempts) {
				ssize_t n = c.read_once();
				if (n < 0)
					throw types::BAD_REQUEST;
				if (n == 0)
					++attempts;
				else
					return;
			}
			throw on_timeout;
		}

				// Implementation of struct __start_line methods

		void	__start_line::query_validate() const {
			const std::string	allowed = "-._~!$&'()*+,;=:/?";
			if (query.empty())
				return;
			size_t size = query.length();
			if (size > QueryLength)
				throw types::BAD_REQUEST;
			for (size_t	i = 0; i < size; ++i) {
				if (query[i] == '%') {
					if (i + 2 < size && std::isxdigit(query[++i]) && std::isxdigit(query[++i]))
						continue;
					else
						throw types::BAD_REQUEST;
				}
				if (!std::isalnum(query[i]) && (allowed.find(query[i]) == std::string::npos))
					throw types::BAD_REQUEST;
			}
		}

		void	__start_line::parse_protocol(const std::string& line, size_t start, size_t end) {
			size_t	protocol_sep = line.find_first_of('/', start);
			if (protocol_sep == std::string::npos)
				throw types:: BAD_REQUEST;
			if (line.compare(start, protocol_sep - start, "HTTP") == 0
					&& (line.compare(protocol_sep + 1, end - protocol_sep, "1.0") == 0
						|| line.compare(protocol_sep + 1, end - protocol_sep, "1.1") == 0))
				version = line.substr(start, end - start + 1);
			else
				throw types::HTTP_VERSION_NOT_SUPPORTED;
		}


		// Implementation of struct __header_map methods

		// static util function
		static std::string trim(const std::string& str) {
			size_t start = str.find_first_not_of(" \t\r\n");
			if (start == std::string::npos)
				return "";
			size_t end = str.find_last_not_of(" \t\r\n");
			return str.substr(start, end - start + 1);
		}

		std::set<std::string>   __header_map::make_unique_headers_list() {
			std::set<std::string> s;
    		s.insert("host");
    		s.insert("content-length");
    		s.insert("content-type");
    		s.insert("authorization");
    		s.insert("user-agent");
    		s.insert("referer");
    		s.insert("content-encoding");
    		s.insert("content-language");
    		s.insert("content-location");
    		s.insert("content-range");
    		s.insert("content-md5");
    		s.insert("date");
    		s.insert("expect");
    		s.insert("from");
    		s.insert("location");
    		s.insert("max-forwards");
    		s.insert("server");
    		s.insert("te");
    		s.insert("upgrade");
    		s.insert("via");
    		return s;
		}

		const std::set<std::string> __header_map::uniques = make_unique_headers_list();

		std::vector<std::string>	__header_map::parse_values(const std::string& values) const {
			std::vector<std::string>	result;
			size_t start_pos = 0;
			while (true) {
				size_t end_pos = values.find_first_of(',', start_pos);
				if (end_pos == std::string::npos) {
					result.push_back(trim(values.substr(start_pos)));
					return result;
				}
				else {
					result.push_back(trim(values.substr(start_pos, end_pos - start_pos)));
					start_pos = end_pos + 1;
				}
			}
			return result;
		}

		static bool	is_cookie_token_char(char c) {
			if (std::isalnum(static_cast<unsigned char>(c)))
				return true;
			const std::string allowed = "!#$%&'*+-.^_`|~";
			return allowed.find(c) != std::string::npos;
		}

		void	__header_map::parse_cookie_line(const std::string& line) {
			size_t start = 0;
			while (start < line.length()) {
				size_t end = line.find(';', start);
				if (end == std::string::npos)
					end = line.length();
				std::string part = trim(line.substr(start, end - start));
				if (!part.empty()) {
					size_t eq = part.find('=');
					if (eq == std::string::npos || eq == 0)
						throw types::BAD_REQUEST;
					std::string name = trim(part.substr(0, eq));
					std::string value = trim(part.substr(eq + 1));
					if (name.empty())
						throw types::BAD_REQUEST;
					for (size_t j = 0; j < name.length(); ++j) {
						if (!is_cookie_token_char(name[j]))
							throw types::BAD_REQUEST;
					}
					cookies[name] = value;
				}
				if (end == line.length())
					break;
				start = end + 1;
			}
		}

		void	__header_map::parse_header(const std::string& line, size_t& len) {
			size_t single_len = line.length();
			len += single_len;
			if (single_len > SingleHeaderLength || len > HeadersLength)
				throw types::REQUEST_HEADER_FIELDS_TOO_LARGE;
			size_t	header_divider = line.find_first_of(':');
			if (header_divider == std::string::npos)
				throw types::BAD_REQUEST;
			std::string	key = line.substr(0, header_divider);
			std::transform(key.begin(), key.end(), key.begin(), static_cast<int(*)(int)>(std::tolower));
			std::vector<std::string> values = parse_values(line.substr(header_divider + 1));
			if (uniques.find(key) != uniques.end() && (header_map.find(key) != header_map.end() || values.size() > 1))
					throw types::BAD_REQUEST;
			if (key == "cookie")
				for (size_t i = 0; i < values.size(); ++i)
					parse_cookie_line(values[i]);
			std::vector<std::string>& headerValues = header_map[key];
			headerValues.insert(headerValues.end(), values.begin(), values.end());
			std::sort(headerValues.begin(), headerValues.end());
			headerValues.erase(std::unique(headerValues.begin(), headerValues.end()), headerValues.end());
		}


		// Implementation of struct __body methos

		size_t	__body::append_to(Connection& c, std::string& target, const std::string& data, size_t take) {
			size_t buffered_lenght = data.length();
			if (buffered_lenght < take)
				take = buffered_lenght;
			target.append(data, 0, take);
			c.consume_read(take);
			return take;
		}

		void	__body::try_to_read(Connection& c) const {
			read_with_attempts(c, types::BAD_REQUEST);
		}

		void    __body::fixed_read(Connection& c, size_t content_len) {
			size_t readed = 0;
			while (readed < content_len) {
				const std::string& buffered = c.read_buffer();
				if (!buffered.empty())
					readed += append_to(c, content, buffered, content_len - readed);
				else
					try_to_read(c);
			}
		}

		void	__body::check_end_of(Connection& c) const {
			std::string result;
			while (result.length() < 2) {
				const std::string& buffered = c.read_buffer();
				if (buffered.empty())
					try_to_read(c);
				else {
					size_t take = 2 - result.length();
					result.append(buffered, 0, take);
					c.consume_read(take);
				}
			}
			if (result[0] != '\r' || result[1] != '\n')
				throw types::BAD_REQUEST;
		}

		void    __body::chunked_read(Connection& c, const std::string& value) {
			if (value != "chunked")
				throw types::NOT_IMPLEMENTED;
			for (size_t chunk_size = get_chunk_size(c); chunk_size; chunk_size = get_chunk_size(c)) {
				fixed_read(c, chunk_size);
				check_end_of(c);
			}
			check_end_of(c);
		}

		size_t	__body::get_chunk_size(Connection& c) {
			std::string line;
			while (true) {
				size_t line_end = line.find("\r\n");
				if (line_end != std::string::npos) {
					std::string size_str = line.substr(0, line_end);
					if (size_str.empty())
						throw types::BAD_REQUEST;
					size_t to_consume = line_end + 2;
					c.consume_read(to_consume);
					try {
						return atoul_base<HEXDECIMAL>(size_str);
					}
					catch (std::logic_error&) {
						throw types::BAD_REQUEST;
					}
				}
				const std::string& buffered = c.read_buffer();
				if (buffered.empty())
					try_to_read(c);
				else {
					if (!line.empty() && line[line.length() - 1] == '\r' && buffered[0] == '\n') {
						line.erase(line.length() - 1);
						c.consume_read(1);
						if (line.empty())
							throw types::BAD_REQUEST;
						try {
							return atoul_base<HEXDECIMAL>(line);
						}
						catch (std::logic_error&) {
							throw types::BAD_REQUEST;
						}
					}
					size_t line_end = buffered.find("\r\n");
					if (line_end == std::string::npos) {
						line.append(buffered);
						c.consume_read(buffered.length());
					}
					else {
						line.append(buffered, 0, line_end);
						c.consume_read(line_end + 2);
						if (line.empty())
							throw types::BAD_REQUEST;
						try {
							return atoul_base<HEXDECIMAL>(line);
						}
						catch (std::logic_error&) {
							throw types::BAD_REQUEST;
						}
					}
				}
			}
		}

		// Implementation class Request methods

		std::map<std::string, std::vector<std::string> >::const_iterator	Request::check_mandatory_headers() const {
			std::map<std::string, std::vector<std::string> >::const_iterator iter;
			if (start_line.version == "HTTP/1.1"
					&& headers.header_map.find("host") == headers.header_map.end())
				throw types::BAD_REQUEST;
			iter = headers.header_map.find("transfer-encoding");
			if (iter == headers.header_map.end())
				iter = headers.header_map.find("content-length");
			if (start_line.method == types::POST && iter == headers.header_map.end())
				throw types::LENGTH_REQUIRED;
			return iter;
		}

		void	Request::parse_start_line(const std::string& line) {
			std::string normalized = line;
			if (!normalized.empty() && normalized[normalized.length() - 1] == '\r')
				normalized.erase(normalized.length() - 1);
			start_line.critical_cases_check(normalized);
			size_t	start_index = 0, end_index = normalized.find_first_of(' ');
			if (!end_index || end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_line.parse_method(normalized, start_index, end_index);
			start_index = end_index + 1;
			end_index = normalized.find_first_of(' ', start_index);
			if (end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_line.parse_uri(normalized, start_index, end_index);
			start_index = end_index + 1;
			end_index = normalized.find_last_not_of("\r\n\0");
			if (end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_line.parse_protocol(normalized, start_index, end_index);
		}

		std::pair<types::HttpStatus, Request> Request::parse_message(Connection& c) {
			Request	parsed_request;
			types::HttpStatus status_code = types::OK;
			try {
				std::string raw = c.read_buffer();
				while (raw.find("\r\n\r\n") == std::string::npos) {
					read_with_attempts(c, types::REQUEST_TIMEOUT);
					raw = c.read_buffer();
				}
				std::stringstream   message_stream(raw);
				std::string	single_line;
				size_t consumed = 0;
				std::getline(message_stream, single_line);
				consumed += single_line.length() + 1;
				parsed_request.parse_start_line(single_line);
				size_t	headers_length = 0;
				while (std::getline(message_stream, single_line)) {
					consumed += single_line.length() + 1;
					if (single_line.empty() || single_line[0] == '\r')
						break ;
					parsed_request.headers.parse_header(single_line, headers_length);
				}
				c.consume_read(consumed);
				std::map<std::string, std::vector<std::string> >::const_iterator body_parsing_mode = parsed_request.check_mandatory_headers();
				if (body_parsing_mode != parsed_request.headers.header_map.end()) {
					if (body_parsing_mode->second.empty())
						throw types::BAD_REQUEST;
					else if ((body_parsing_mode->first)[0] == 'c')
						parsed_request.body.fixed_read(c, atoul_base<DECIMAL>(body_parsing_mode->second.front()));
					else
						parsed_request.body.chunked_read(c, body_parsing_mode->second.front());
				}
			}
			catch(types::HttpStatus n) { status_code = n; }
			catch(...) { status_code = types::INTERNAL_SERVER_ERROR; }
			return std::make_pair(status_code, parsed_request);
		}

	}
}
