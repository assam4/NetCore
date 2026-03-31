#include "http_response.hpp"
#include <fstream>
#include <ctime>
#include <sstream>

namespace http {
	namespace core {

		StatusRegistry::StatusRegistry() {
			_phrases = {
				// 1xx - Informational
				{types::CONTINUE, "Continue"},
				{types::SWITCHING_PROTOCOLS, "Switching Protocols"},
				{types::PROCESSING, "Processing"},
				{types::EARLY_HINTS, "Early Hints"},
				// 2xx - Success
				{types::OK, "OK"},
				{types::CREATED, "Created"},
				{types::ACCEPTED, "Accepted"},
				{types::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
				{types::NO_CONTENT, "No Content"},
				{types::RESET_CONTENT, "Reset Content"},
				{types::PARTIAL_CONTENT, "Partial Content"},
				{types::MULTI_STATUS, "Multi-Status"},
				{types::ALREADY_REPORTED, "Already Reported"},
				{types::IM_USED, "IM Used"},
				// 3xx - Redirection
				{types::MULTIPLE_CHOICES, "Multiple Choices"},
				{types::MOVED_PERMANENTLY, "Moved Permanently"},
				{types::FOUND, "Found"},
				{types::SEE_OTHER, "See Other"},
				{types::NOT_MODIFIED, "Not Modified"},
				{types::USE_PROXY, "Use Proxy"},
				{types::UNUSED, "Unused"},
				{types::TEMPORARY_REDIRECT, "Temporary Redirect"},
				{types::PERMANENT_REDIRECT, "Permanent Redirect"},
				// 4xx - Client Errors
				{types::BAD_REQUEST, "Bad Request"},
				{types::UNAUTHORIZED, "Unauthorized"},
				{types::PAYMENT_REQUIRED, "Payment Required"},
				{types::FORBIDDEN, "Forbidden"},
				{types::NOT_FOUND, "Not Found"},
				{types::METHOD_NOT_ALLOWED, "Method Not Allowed"},
				{types::NOT_ACCEPTABLE, "Not Acceptable"},
				{types::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
				{types::REQUEST_TIMEOUT, "Request Timeout"},
				{types::CONFLICT, "Conflict"},
				{types::GONE, "Gone"},
				{types::LENGTH_REQUIRED, "Length Required"},
				{types::PRECONDITION_FAILED, "Precondition Failed"},
				{types::CONTENT_TOO_LARGE, "Content Too Large"},
				{types::URI_TOO_LOONG, "URI Too Long"},
				{types::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
				{types::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
				{types::EXPECTATION_FAILED, "Expectation Failed"},
				{types::I_AM_NOT_TEAPOT, "I'm a teapot"},
				{types::MISDIRECTED_REQUEST, "Misdirected Request"},
				{types::UNPROCESSABLE_CONTENT, "Unprocessable Content"},
				{types::LOCKED, "Locked"},
				{types::FAILED_DEPENDENCY, "Failed Dependency"},
				{types::TOO_EARLY, "Too Early"},
				{types::UPGRADE_REQUIRED, "Upgrade Required"},
				{types::PRECONDITION_REQUIRED, "Precondition Required"},
				{types::TOO_MANY_REQUESTS, "Too Many Requests"},
				{types::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
				{types::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},
				// 5xx - Server Errors
				{types::INTERNAL_SERVER_ERROR, "Internal Server Error"},
				{types::NOT_IMPLEMENTED, "Not Implemented"},
				{types::BAD_GATEWAY, "Bad Gateway"},
				{types::SERVICE_UNAVAILABLE, "Service Unavailable"},
				{types::GATEWAY_TIMEOUT, "Gateway Timeout"},
				{types::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
				{types::VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
				{types::INSUFFICIENT_STORAGE, "Insufficient Storage"},
				{types::LOOP_DETECTED, "Loop Detected"},
				{types::NOT_EXTENDED, "Not Extended"},
				{types::NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required"}
			};
		}

		StatusRegistry& StatusRegistry::instance() {
			static StatusRegistry obj;
			return obj;
		}

		std::string StatusRegistry::get_phrase(types::HttpStatus status) const{
			std::map<types::HttpStatus, std::string>::const_iterator it = _phrases.find(status);
			if (it != _phrases.end())
				return it->second;
			return "Unknown";
		}

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
			std::string path = "../utils/" + ss.str() + ".html";
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
				if (body_known) {
					std::stringstream ss;
					ss << res._body.size();
					res._headers["Content-Length"] = ss.str();
				} else {
					if (connection == "keep-alive")
						res._headers["Transfer-Encoding"] = "chunked";
				}
			} else {
				if (body_known) {
					std::stringstream ss;
					ss << res._body.size();
					res._headers["Content-Length"] = ss.str();
				} else {
					res._headers["Connection"] = "close";
				}
			}
		}

		Response::_http_response Response::make_response(const std::pair<types::HttpStatus, Request::__http_request>& req) {
			Response::_http_response res;

			res._version = req.second.version;
			set_connection_field(res, req.second);
			res.
		}
	}
}
