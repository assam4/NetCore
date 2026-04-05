#include "http_request.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

namespace http {
	namespace core {

		const static size_t RequestLineLength = 8192; // 8KB
		const static size_t URILength = 4096; // 4KB
		const static size_t SingleHeaderLength = 8192; // 8KB
		const static size_t HeadersLength = 32768; // 32KB
		const static size_t DefaultMaxBodyLength = 1024;// 1KB

		void    Request::parse_method(Request::__http_request& hr, const std::string& line, size_t& position) {
			if (line.compare(0, 3, "GET") == 0 && line.size() > 3 && line[3] == ' ') {
				hr.method = types::GET;
				position = 3;
			}
			else if (line.compare(0, 4, "POST") == 0 && line.size() > 4 && line[4] == ' ') {
				hr.method = types::POST;
				position = 4;
			}
			else if (line.compare(0, 6, "DELETE") == 0 && line.size() > 6 && line[6] == ' ') {
				hr.method = types::DEL;
				position = 6;
			}
			else
				throw types::NOT_IMPLEMENTED;
		}

		bool Request::is_query_char(unsigned char c) {
			if (std::isalnum(c))
				return true;
			switch (c) {
				case '-': case '.': case '_': case '~':
				case '!': case '$': case '&': case '\'':
				case '(': case ')': case '*': case '+':
				case ',': case ';': case '=':
				case ':': case '@':
				case '/': case '?':
					return true;
				default:
					return false;
			}
		}

		void Request::validate_query_string(const std::string& query) {
			for (size_t i = 0; i < query.length(); ++i) {
				unsigned char c = query[i];
				if (c == '%') {
					if (i + 2 >= query.length() || !isxdigit(query[i+1]) || !std::isxdigit(query[i + 2]))
						throw types::BAD_REQUEST;
					i += 2;
					continue;
				}
				if (!is_query_char(c))
					throw types::BAD_REQUEST;
			}
		}

		void Request::validate_uri(Request::__http_request& hr) {
			size_t position = 0;
			if (hr.uri.compare(0, 7, "http://") == 0)
				position = 7;
			if (position) {
				size_t end_pos = hr.uri.find_first_of('/', position);
				if (end_pos == std::string::npos)
					throw types::BAD_REQUEST;
				position = end_pos;
			}
			if (hr.uri[position] != '/')
				throw types::BAD_REQUEST;
			size_t  query_start = hr.uri.find_first_of('?', position);
			if (query_start != std::string::npos) {
				hr.query = hr.uri.substr(query_start + 1);
				hr.uri.erase(query_start);
				validate_query_string(hr.query);
			}
		}

		void    Request::parse_uri(Request::__http_request& hr, const std::string& line, size_t& position) {
			if (position >= line.size() || !std::isspace(line[position]))
				throw types::BAD_REQUEST;
			++position;
			size_t  end_pos = line.find_first_of(' ', position);
			if (end_pos == std::string::npos)
				throw types::BAD_REQUEST;
			hr.uri = line.substr(position, end_pos - position);
			position = end_pos + 1;
			if (hr.uri.empty())
				throw types::BAD_REQUEST;
			if (hr.uri.length() > URILength)
				throw types::URI_TOO_LOONG;
			validate_uri(hr);
		}

		void    Request::parse_protocol(Request::__http_request& hr, const std::string& line, size_t& position) {
			size_t  end_pos = line.find_first_of('/', position);
			if (end_pos == std::string::npos)
				throw types::BAD_REQUEST;
			if (line.compare(position, end_pos - position, "HTTP") != 0)
				throw types::HTTP_VERSION_NOT_SUPPORTED;
			++end_pos;
			hr.version = line.substr(end_pos, line.length() - end_pos);
			if (!hr.version.empty() && hr.version[hr.version.size() - 1] == '\r')
				hr.version.erase(hr.version.size() - 1);
			size_t last = hr.version.find_last_not_of(" \t");
			if (last != std::string::npos)
				hr.version.erase(last + 1);
			else
				hr.version.clear();
			if (hr.version != "1.0" && hr.version != "1.1")
				throw types::HTTP_VERSION_NOT_SUPPORTED;
		}

		void    Request::parse_start_line(std::stringstream& os, Request::__http_request& hr) {
			size_t  position = 0;
			std::string line;
			std::getline(os, line);
			if (line.empty())
				throw types::BAD_REQUEST;
			if (line.length() > RequestLineLength)
				throw types::URI_TOO_LOONG;
			parse_method(hr, line, position);
			parse_uri(hr, line, position);
			parse_protocol(hr, line, position);
		}

		void    Request::parse_headers(std::stringstream& os, Request::__http_request& hr) {
			size_t  length = 0;
			bool    host_status = (hr.version == "1.1") ? false : true;
			std::string line;
			while (std::getline(os, line)) {
				if (!line.empty() && line[line.size()-1] == '\r')
					line.erase(line.size()-1);
				if (line.empty())
					break;
				length += line.length();
				if (length > HeadersLength || line.length() > SingleHeaderLength)
					throw types::REQUEST_HEADER_FIELDS_TOO_LARGE;
				if (std::isspace(line[0]))
					throw types::BAD_REQUEST;
				size_t sep = line.find_first_of(":");
				if (sep == std::string::npos)
					throw types::BAD_REQUEST;
				std::string key = line.substr(0, sep);
				for (size_t i = 0; i < key.size(); ++i)
					key[i] = std::tolower(key[i]);
				key.erase(key.find_last_not_of(" \t\r\n") + 1);
				++sep;
				std::string value = (sep < line.size()) ? line.substr(sep) : std::string("");
				value.erase(0, value.find_first_not_of(" \t\r\n"));
				value.erase(value.find_last_not_of(" \t\r\n") + 1);
				if (!host_status && (key == "host"))
					host_status = true;
				(hr.headers[key]).push_back(value);
			}
			if (!host_status)
				throw types::BAD_REQUEST;
		}

		void    Request::parse_body(std::stringstream& os, Request::__http_request& hr, size_t max_body_size) {
			std::string body((std::istreambuf_iterator<char>(os)), std::istreambuf_iterator<char>());
			size_t pos = 0;
			while (pos < body.size() && (body[pos] == '\r' || body[pos] == '\n'))
				++pos;
            if (pos > max_body_size)
                throw types::CONTENT_TOO_LARGE;
			hr.body = body.substr(pos);
		}

		std::pair<types::HttpStatus, Request::__http_request> Request::parse_message(const std::string& message, size_t max_body_size) {
			Request::__http_request  hr;
			types::HttpStatus status_code = types::OK;
			try {
				std::stringstream   os;
				os << message;
				parse_start_line(os, hr);
				parse_headers(os, hr);
				parse_body(os, hr, max_body_size);
			}
			catch(types::HttpStatus n) {
				status_code = n;
			}
			catch(...) {
				status_code = types::INTERNAL_SERVER_ERROR;
			}
			return std::make_pair(status_code, hr);
		}
	}
}
