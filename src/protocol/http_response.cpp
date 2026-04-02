#include "http_response.hpp"
#include <fstream>
#include <ctime>
#include "to_string.hpp"
#include "http_types.hpp"
#include <sstream>

namespace http {
	namespace core {

		std::string Response::read_file(const std::string& path) {
			std::ifstream file(path.c_str());
			if (!file.is_open())
				return "";
			std::stringstream ss;
			ss << file.rdbuf();
			return ss.str();
		}

		void Response::make_error(_http_response& res,types::HttpStatus status) {
			if (status != types::OK)
				res._status = status;
			res._headers["Content-Type"] = "text/html";
			std::stringstream ss;
			ss << status;
			std::string path = "../../error/" + ss.str() + ".html";
			std::string body = read_file(path);
			if (body.empty()) {
				ss.clear();
				ss << "<html><body><h1>" << status << " Error</h1></body></html>";
				body = ss.str();
			}
			res._body = body;
		}

		void Response::set_connection_field(_http_response& res, const Request::__http_request& req) {
			std::string conn_value;
			std::map<std::string, std::vector<std::string> >::const_iterator it = req.headers.find("Connection");
			if (it != req.headers.end()) {
				bool has_close = false;
				bool has_keep_alive = false;
				for (std::vector<std::string>::const_iterator vit = it->second.begin(); vit != it->second.end(); ++vit) {
					std::string token = *vit;
					for (size_t i = 0; i < token.size(); ++i)
						token[i] = tolower(token[i]);
					if (token.find("close") != std::string::npos)
						has_close = true;
					else if (token.find("keep-alive") != std::string::npos)
						has_keep_alive = true;
				}
				if (has_close)
					conn_value = "close";
				else if (has_keep_alive && req.version == "HTTP/1.1")
					conn_value = "keep-alive";
				else
					conn_value = "close";
			} else {
				if (req.version == "HTTP/1.1")
					conn_value = "keep-alive";
				else
					conn_value = "close";
			}
			res._headers["Connection"] = conn_value;
		}

		void Response::set_server_field(_http_response& res) {
			res._headers["Server"] = "webserv";
		}

		void Response::set_date_field(Response::_http_response& res) {
			std::time_t t = std::time(NULL);
			std::tm* gmt = std::gmtime(&t);
			char date[100];
			std::strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);
			res._headers["Date"] = std::string(date);
		}

		void Response::set_body_length_field(Response::_http_response& res) {
			bool is_http_1_1 = (res._version == "HTTP/1.1");
			std::string connection = "close";
			std::map<std::string, std::string>::const_iterator it = res._headers.find("Connection");
			if (it != res._headers.end())
				connection = it->second;
			bool body_known = !res._body.empty();
			if (is_http_1_1) {
				if (body_known)
					res._headers["Content-Length"] = to_string(res._body.size());
				else {
					if (connection == "keep-alive")
						res._headers["Transfer-Encoding"] = "chunked";
				}
			} else {
				if (body_known)
					res._headers["Content-Length"] = to_string(res._body.size());
				else
					res._headers["Connection"] = "close";
			}
		}

		void Response::set_content_type_field(Response::_http_response& res, const std::string& path) {
			std::string ext;
			size_t pos = path.find_last_of('.');
			if (pos != std::string::npos && pos + 1 < path.size())
				ext = path.substr(pos + 1);
			std::string mime_type = types::MimeTypes::get_mime_type(ext);
			res._headers["Content-Type"] = mime_type;
		}

		void Response::set_allow_field(Response::_http_response& res, uint8_t methods) {
			std::string list;
			if (methods & types::GET)
				list += "GET";
			if (methods & types::POST) {
				if (!list.empty())
					list += ", ";
				list += "POST";
			}
			if (methods & types::DEL) {
				if (!list.empty())
					list += ", ";
				list += "DELETE";
			}
			res._headers["Allow"] = list;
		}

	}
}
