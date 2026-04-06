#include "http_request.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace http {
	namespace core {

		const static size_t StartLineLength = 8192; // 8KB
		const static size_t URILength = 4096; // 4KB
		const static size_t	QueryLength = 2048; // 2KB
		const static size_t SingleHeaderLength = 8192; // 8KB
		const static size_t HeadersLength = 32768; // 32KB
		const static size_t DefaultMaxBodyLength = 1024;// 1KB

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
				version = line.substr(start, end - start);
			else
				throw types::HTTP_VERSION_NOT_SUPPORTED;
		}

		void	Request::parse_start_line(const std::string& line) {
			start_line.critical_cases_check(line);
			size_t	start_index = 0, end_index = line.find_first_of(' ');
			if (!end_index || end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_line.parse_method(line, start_index, end_index);
			start_index = end_index + 1;
			end_index = line.find_first_of(' ', start_index);
			if (end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_index = end_index + 1;
			end_index = line.find_last_not_of("\r\n\0");
			if (end_index == std::string::npos)
				throw types::BAD_REQUEST;
			start_line.parse_protocol(line, start_index, end_index);	
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

		static std::string trim(const std::string& str) {
			size_t start = str.find_first_not_of(" \t");
			if (start == std::string::npos)
				return "";
			size_t end = str.find_last_not_of(" \t");
			return str.substr(start, end - start + 1);
		}

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
			std::vector<std::string>& headerValues = header_map[key];
			headerValues.insert(headerValues.end(), values.begin(), values.end());
			std::sort(headerValues.begin(), headerValues.end());
			headerValues.erase(std::unique(headerValues.begin(), headerValues.end()), headerValues.end());
		}

		void	Request::check_mandatory_headers() const {
			if (start_line.version == "HTTP/1.1"
					&& headers.header_map.find("host") == headers.header_map.end())
				throw types::BAD_REQUEST;
			if ((start_line.method == types::POST)
					&& headers.header_map.find("content-length") == headers.header_map.end()
					&& headers.header_map.find("transfer-encoding") == headers.header_map.end())
				throw types::LENGTH_REQUIRED;
		}

		std::pair<types::HttpStatus, Request> Request::parse_message(const std::string& message) {
			Request	parsed_request;
			types::HttpStatus status_code = types::OK;
			try {
				std::stringstream   message_stream(message);
				std::string	single_line;
				std::getline(message_stream, single_line);
				parsed_request.parse_start_line(single_line);
				size_t	headers_length = 0;
				while (std::getline(message_stream, single_line)) {
					if (single_line.empty() || single_line[0] == '\r')
						break ;
					parsed_request.headers.parse_header(single_line, headers_length);
				}
				parsed_request.check_mandatory_headers();
			}
			catch(types::HttpStatus n) { status_code = n; }
			catch(...) { status_code = types::INTERNAL_SERVER_ERROR; }
			return std::make_pair(status_code, parsed_request);
		}
	}
}
