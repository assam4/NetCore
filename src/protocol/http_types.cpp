#include "http_types.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace http {
	namespace core {
		namespace types {

			std::map<types::HttpStatus, std::string> StatusRegistry::_phrases;
			bool StatusRegistry::_initialized = false;
			std::map<std::string, std::string> MimeTypes::_types;

			void StatusRegistry::initialize() {
				if (_initialized)
					return;
				_initialized = true;
				// 1xx - Informational
				_phrases[types::CONTINUE] = "Continue";
				_phrases[types::SWITCHING_PROTOCOLS] = "Switching Protocols";
				_phrases[types::PROCESSING] = "Processing";
				_phrases[types::EARLY_HINTS] = "Early Hints";
				// 2xx - Success
				_phrases[types::OK] = "OK";
				_phrases[types::CREATED] = "Created";
				_phrases[types::ACCEPTED] = "Accepted";
				_phrases[types::NON_AUTHORITATIVE_INFORMATION] = "Non-Authoritative Information";
				_phrases[types::NO_CONTENT] = "No Content";
				_phrases[types::RESET_CONTENT] = "Reset Content";
				_phrases[types::PARTIAL_CONTENT] = "Partial Content";
				_phrases[types::MULTI_STATUS] = "Multi-Status";
				_phrases[types::ALREADY_REPORTED] = "Already Reported";
				_phrases[types::IM_USED] = "IM Used";
				// 3xx - Redirection
				_phrases[types::MULTIPLE_CHOICES] = "Multiple Choices";
				_phrases[types::MOVED_PERMANENTLY] = "Moved Permanently";
				_phrases[types::FOUND] = "Found";
				_phrases[types::SEE_OTHER] = "See Other";
				_phrases[types::NOT_MODIFIED] = "Not Modified";
				_phrases[types::USE_PROXY] = "Use Proxy";
				_phrases[types::UNUSED] = "Unused";
				_phrases[types::TEMPORARY_REDIRECT] = "Temporary Redirect";
				_phrases[types::PERMANENT_REDIRECT] = "Permanent Redirect";
				// 4xx - Client Errors
				_phrases[types::BAD_REQUEST] = "Bad Request";
				_phrases[types::UNAUTHORIZED] = "Unauthorized";
				_phrases[types::PAYMENT_REQUIRED] = "Payment Required";
				_phrases[types::FORBIDDEN] = "Forbidden";
				_phrases[types::NOT_FOUND] = "Not Found";
				_phrases[types::METHOD_NOT_ALLOWED] = "Method Not Allowed";
				_phrases[types::NOT_ACCEPTABLE] = "Not Acceptable";
				_phrases[types::PROXY_AUTHENTICATION_REQUIRED] = "Proxy Authentication Required";
				_phrases[types::REQUEST_TIMEOUT] = "Request Timeout";
				_phrases[types::CONFLICT] = "Conflict";
				_phrases[types::GONE] = "Gone";
				_phrases[types::LENGTH_REQUIRED] = "Length Required";
				_phrases[types::PRECONDITION_FAILED] = "Precondition Failed";
				_phrases[types::CONTENT_TOO_LARGE] = "Content Too Large";
				_phrases[types::URI_TOO_LOONG] = "URI Too Long";
				_phrases[types::UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
				_phrases[types::RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
				_phrases[types::EXPECTATION_FAILED] = "Expectation Failed";
				_phrases[types::I_AM_NOT_TEAPOT] = "I'm a teapot";
				_phrases[types::MISDIRECTED_REQUEST] = "Misdirected Request";
				_phrases[types::UNPROCESSABLE_CONTENT] = "Unprocessable Content";
				_phrases[types::LOCKED] = "Locked";
				_phrases[types::FAILED_DEPENDENCY] = "Failed Dependency";
				_phrases[types::TOO_EARLY] = "Too Early";
				_phrases[types::UPGRADE_REQUIRED] = "Upgrade Required";
				_phrases[types::PRECONDITION_REQUIRED] = "Precondition Required";
				_phrases[types::TOO_MANY_REQUESTS] = "Too Many Requests";
				_phrases[types::REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
				_phrases[types::UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons";
				// 5xx - Server Errors
				_phrases[types::INTERNAL_SERVER_ERROR] = "Internal Server Error";
				_phrases[types::NOT_IMPLEMENTED] = "Not Implemented";
				_phrases[types::BAD_GATEWAY] = "Bad Gateway";
				_phrases[types::SERVICE_UNAVAILABLE] = "Service Unavailable";
				_phrases[types::GATEWAY_TIMEOUT] = "Gateway Timeout";
				_phrases[types::HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
				_phrases[types::VARIANT_ALSO_NEGOTIATES] = "Variant Also Negotiates";
				_phrases[types::INSUFFICIENT_STORAGE] = "Insufficient Storage";
				_phrases[types::LOOP_DETECTED] = "Loop Detected";
				_phrases[types::NOT_EXTENDED] = "Not Extended";
				_phrases[types::NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required";
			}

			StatusRegistry::StatusRegistry() {
			}

			std::string StatusRegistry::get_phrase(types::HttpStatus status) {
				initialize();
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
					if (!ext.empty() && ext[ext.length() - 1] == ';')
						ext.erase(ext.length() - 1);
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
