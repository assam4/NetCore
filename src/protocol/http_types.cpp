#include "http_types.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace http {
	namespace core {
		namespace types {

			StatusRegistry::StatusRegistry() {
				const std::map<types::HttpStatus, std::string> _phrases = {
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
					{types::URI_TOO_LONG, "URI Too Long"},
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

			std::string StatusRegistry::get_phrase(types::HttpStatus status) {
				std::map<types::HttpStatus, std::string>::const_iterator it = _phrases.find(status);
				if (it != _phrases.end())
					return it->second;
				return "Unknown";
			}

			bool MimeTypes::parse_mime_line(const std::string& line) {
				std::istringstream iss(line);
				std::string mime_type;
				iss >> mime_type;
				if (mime_type.empty())
					return false;
				std::string ext;
				bool added = false;
				while (iss >> ext) {
					if (!ext.empty() && ext.back() == ';')
						ext.pop_back();
					_types[ext] = mime_type;
					added = true;
				}
				return added;
			}

			void MimeTypes::setup_mime_types() {
				const std::string path = "../../config/mime.types";
				std::ifstream file(path.c_str());
				if (!file.is_open()) {
					std::cerr << "You must have mime.types file inside config\n";
					return;
				}
				std::string line;
				bool types_block_started = false;
				bool types_block_closed = false;
				while (std::getline(file, line)) {
					line.erase(0, line.find_first_not_of(" \t"));
					line.erase(line.find_last_not_of(" \t\r\n") + 1);
					if (line.empty() || line[0] == '#')
						continue;
					if (!types_block_started) {
					if (line == "types {" || line == "types{") {
							types_block_started = true;
							continue;
						} else {
							std::cerr << "Invalid mime.types format: missing types block\n";
							return;
						}
					}
					if (line == "}") {
						if (_types.empty()) {
							std::cerr << "Invalid mime.types format: types block is empty\n";
							return;
						}
						types_block_closed = true;
						break;
					}
					if (line.find("types") != std::string::npos) {
						std::cerr << "Invalid mime.types format: nested types block detected\n";
						return;
					}
					if (!parse_mime_line(line)) {
						std::cerr << "Invalid mime.types line (no extensions): " << line << "\n";
						return;
					}
				}
				if (!types_block_started || !types_block_closed)
					std::cerr << "Invalid mime.types format: types block not properly closed\n";
			}

			std::string MimeTypes::get_mime_type(const std::string& ext) {
				std::map<std::string, std::string>::const_iterator it = _types.find(ext);
				if (it != _types.end())
					return it->second;
				return "application/octet-stream";
			}

		}
	}
}
