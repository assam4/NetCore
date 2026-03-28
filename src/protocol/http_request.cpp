#include "http_request.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

namespace http {
    namespace core {

        void    Request::parse_method(Request::__http_request& hr, const std::string& line, size_t& position) {
            if (line.compare(0, 3, "GET") == 0) {
                hr.method = types::GET;
                position = 3;
            }
            else if (line.compare(0, 4, "POST") == 0) {
                hr.method = types::POST;
                position = 4;
            }
            else if (line.compare(0, 3, "DEL") == 0 || line.compare(0, 6, "DELETE") == 0) {
                hr.method = types::DEL;
                position = (line[3] == 'E') ? 6 : 3;
            }
            else
                throw std::invalid_argument("Request error: Invalid request method(unauthorized API).\n");
        }

        void    Request::parse_uri(Request::__http_request& hr, const std::string& line, size_t& position) {
            if (!std::isspace(line[position]))
                throw std::invalid_argument("Request error: There is no separator between method and uri!.\n");
            ++position;
            size_t  end_pos = line.find_first_of(' ', position);
            if (end_pos == std::string::npos)
                throw std::invalid_argument("Request error: There is no separator between uri and protocol!.\n");
            if (line[position] != '/')
                throw std::invalid_argument("Request error: invalid uri form!.\n");
            hr.uri = line.substr(position, end_pos - position);
            position = end_pos + 1;
        }

        void    Request::parse_protocol(Request::__http_request& hr, const std::string& line, size_t& position) {
            size_t  end_pos = line.find_first_of('/', position);
            if (end_pos == std::string::npos)
                throw std::invalid_argument("Request error: There is no protocol version!.\n");
            if (line.compare(position, end_pos - position, "HTTP"))
                throw std::invalid_argument("Request error: Unsupported protocol!.\n");
            ++end_pos;
            hr.version = line.substr(end_pos, line.length() - end_pos);
            hr.version.erase(3, 1);
            if (hr.version != "1.0" && hr.version != "1.1")
                throw std::invalid_argument("Request error: Unsupported http protocol version!.\n");
        }

        void    Request::parse_start_line(std::stringstream& os, Request::__http_request& hr) {
            std::string line;
            size_t  position = 0;
            std::getline(os, line);
            if (line.empty())
                throw std::invalid_argument("Request error: empty message!.\n");
            parse_method(hr, line, position);
            parse_uri(hr, line, position);
            parse_protocol(hr, line, position);
        }

        void    Request::parse_headers(std::stringstream& os, Request::__http_request& hr) {
            bool    host_status = (hr.version == "1.1") ? false : true;
            std::string line;
            while (std::getline(os, line)) {
                if (!line.empty() && line[line.size()-1] == '\r')
                    line.erase(line.size()-1);
                if (line.empty())
                    break;
                if (std::isspace(line[0]))
                    throw std::invalid_argument("Request error: invalid header in request1!.\n");
                size_t sep = line.find_first_of(":");
                if (sep == std::string::npos)
                    throw std::invalid_argument("Request error: invalid header in request2!.\n");
                std::string key = line.substr(0, sep);
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                ++sep;
                std::string value = (sep < line.size()) ? line.substr(sep) : std::string("");
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                if (!host_status && (key == "Host"))
                    host_status = true;
                (hr.headers[key]).push_back(value);
            }
            if (!host_status)
                throw std::invalid_argument("Request error: There is no 'Host' header in current HTTP/1.1 protocol.!\n");
        }

        void    Request::parse_body(std::stringstream& os, Request::__http_request& hr) {
            std::string body((std::istreambuf_iterator<char>(os)), std::istreambuf_iterator<char>());
            size_t pos = 0;
            while (pos < body.size() && (body[pos] == '\r' || body[pos] == '\n'))
                ++pos;
            hr.body = body.substr(pos);
            if (hr.method == types::GET && !hr.body.empty())
                throw std::invalid_argument("Request error: Get request can't contain a body!.\n");
            if (hr.method == types::POST && hr.body.empty())
                throw std::invalid_argument("Request error: Post request must have a body!.\n");
        }

        std::pair<uint16_t, Request::__http_request> Request::parse_message(const std::string& message) {
            Request::__http_request  hr;
            uint16_t    status_code = 200;
            try {
                std::stringstream   os;
                os << message;
                parse_start_line(os, hr);
                parse_headers(os, hr);
                parse_body(os, hr);
            }
            catch(const std::invalid_argument& e) {
                std::cerr << e.what() << std::endl;
                status_code = 400;
            }
            catch(...) {
                std::cerr << "Request error: Unknown exception!.\n";
                status_code = 400;
            }
            return std::make_pair(status_code, hr);
        }

    }
}
