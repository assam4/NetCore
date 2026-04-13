#include "http_types.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace http {
	namespace core {
		namespace types {

			const std::map<types::HttpStatus, std::string> StatusRegistry::_phrases = StatusRegistry::init();
			const std::map<std::string, std::string> MimeTypes::_types = MimeTypes::init();

			std::map<types::HttpStatus, std::string> StatusRegistry::init() {
				std::map<types::HttpStatus, std::string> m;
				m[types::CONTINUE] = "Continue";
				m[types::SWITCHING_PROTOCOLS] = "Switching Protocols";
				m[types::PROCESSING] = "Processing";
				m[types::EARLY_HINTS] = "Early Hints";
				m[types::OK] = "OK";
				m[types::CREATED] = "Created";
				m[types::ACCEPTED] = "Accepted";
				m[types::NON_AUTHORITATIVE_INFORMATION] = "Non-Authoritative Information";
				m[types::NO_CONTENT] = "No Content";
				m[types::RESET_CONTENT] = "Reset Content";
				m[types::PARTIAL_CONTENT] = "Partial Content";
				m[types::MULTI_STATUS] = "Multi-Status";
				m[types::ALREADY_REPORTED] = "Already Reported";
				m[types::IM_USED] = "IM Used";
				m[types::MULTIPLE_CHOICES] = "Multiple Choices";
				m[types::MOVED_PERMANENTLY] = "Moved Permanently";
				m[types::FOUND] = "Found";
				m[types::SEE_OTHER] = "See Other";
				m[types::NOT_MODIFIED] = "Not Modified";
				m[types::USE_PROXY] = "Use Proxy";
				m[types::UNUSED] = "Unused";
				m[types::TEMPORARY_REDIRECT] = "Temporary Redirect";
				m[types::PERMANENT_REDIRECT] = "Permanent Redirect";
				m[types::BAD_REQUEST] = "Bad Request";
				m[types::UNAUTHORIZED] = "Unauthorized";
				m[types::PAYMENT_REQUIRED] = "Payment Required";
				m[types::FORBIDDEN] = "Forbidden";
				m[types::NOT_FOUND] = "Not Found";
				m[types::METHOD_NOT_ALLOWED] = "Method Not Allowed";
				m[types::NOT_ACCEPTABLE] = "Not Acceptable";
				m[types::PROXY_AUTHENTICATION_REQUIRED] = "Proxy Authentication Required";
				m[types::REQUEST_TIMEOUT] = "Request Timeout";
				m[types::CONFLICT] = "Conflict";
				m[types::GONE] = "Gone";
				m[types::LENGTH_REQUIRED] = "Length Required";
				m[types::PRECONDITION_FAILED] = "Precondition Failed";
				m[types::CONTENT_TOO_LARGE] = "Content Too Large";
				m[types::URI_TOO_LONG] = "URI Too Long";
				m[types::UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
				m[types::RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
				m[types::EXPECTATION_FAILED] = "Expectation Failed";
				m[types::I_AM_NOT_TEAPOT] = "I'm a teapot";
				m[types::MISDIRECTED_REQUEST] = "Misdirected Request";
				m[types::UNPROCESSABLE_CONTENT] = "Unprocessable Content";
				m[types::LOCKED] = "Locked";
				m[types::FAILED_DEPENDENCY] = "Failed Dependency";
				m[types::TOO_EARLY] = "Too Early";
				m[types::UPGRADE_REQUIRED] = "Upgrade Required";
				m[types::PRECONDITION_REQUIRED] = "Precondition Required";
				m[types::TOO_MANY_REQUESTS] = "Too Many Requests";
				m[types::REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
				m[types::UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons";
				m[types::INTERNAL_SERVER_ERROR] = "Internal Server Error";
				m[types::NOT_IMPLEMENTED] = "Not Implemented";
				m[types::BAD_GATEWAY] = "Bad Gateway";
				m[types::SERVICE_UNAVAILABLE] = "Service Unavailable";
				m[types::GATEWAY_TIMEOUT] = "Gateway Timeout";
				m[types::HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
				m[types::VARIANT_ALSO_NEGOTIATES] = "Variant Also Negotiates";
				m[types::INSUFFICIENT_STORAGE] = "Insufficient Storage";
				m[types::LOOP_DETECTED] = "Loop Detected";
				m[types::NOT_EXTENDED] = "Not Extended";
				m[types::NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required";
				return m;
			}

			const std::string& StatusRegistry::get_phrase(types::HttpStatus status)
			{
				static const std::string unknown = "Unknown";
				std::map<types::HttpStatus, std::string>::const_iterator it = _phrases.find(status);
				if (it != _phrases.end())
					return it->second;
				return unknown;
			}

			std::map<std::string, std::string> MimeTypes::init() {
				std::map<std::string, std::string> m;
				m[".html"] = "text/html";
				m[".htm"]  = "text/html";
				m[".css"]  = "text/css";
				m[".csv"]  = "text/csv";
				m[".txt"]  = "text/plain";
				m[".xml"]  = "application/xml";
				m[".js"]   = "application/javascript";
				m[".json"] = "application/json";
				m[".pdf"]  = "application/pdf";
				m[".zip"]  = "application/zip";
				m[".gz"]   = "application/gzip";
				m[".tar"]  = "application/x-tar";
				m[".rar"]  = "application/vnd.rar";
				m[".7z"]   = "application/x-7z-compressed";
				m[".exe"]  = "application/octet-stream";
				m[".bin"]  = "application/octet-stream";
				m[".png"]  = "image/png";
				m[".jpg"]  = "image/jpeg";
				m[".jpeg"] = "image/jpeg";
				m[".gif"]  = "image/gif";
				m[".bmp"]  = "image/bmp";
				m[".webp"] = "image/webp";
				m[".svg"]  = "image/svg+xml";
				m[".ico"]  = "image/x-icon";
				m[".tif"]  = "image/tiff";
				m[".tiff"] = "image/tiff";
				m[".mp3"]  = "audio/mpeg";
				m[".wav"]  = "audio/wav";
				m[".ogg"]  = "audio/ogg";
				m[".aac"]  = "audio/aac";
				m[".flac"] = "audio/flac";
				m[".mp4"]  = "video/mp4";
				m[".avi"]  = "video/x-msvideo";
				m[".mov"]  = "video/quicktime";
				m[".mkv"]  = "video/x-matroska";
				m[".webm"] = "video/webm";
				m[".wmv"]  = "video/x-ms-wmv";
				m[".flv"]  = "video/x-flv";
				m[".woff"]  = "font/woff";
				m[".woff2"] = "font/woff2";
				m[".ttf"]   = "font/ttf";
				m[".otf"]   = "font/otf";
				m[".eot"]   = "application/vnd.ms-fontobject";
				m[".apk"]  = "application/vnd.android.package-archive";
				m[".iso"]  = "application/x-iso9660-image";
				m[".doc"]  = "application/msword";
				m[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
				m[".xls"]  = "application/vnd.ms-excel";
				m[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
				m[".ppt"]  = "application/vnd.ms-powerpoint";
				m[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
				return m;
			}

			const std::string& MimeTypes::get_mime_type(const std::string& ext) {
				static const std::string default_type = "application/octet-stream";
				std::map<std::string, std::string>::const_iterator it = _types.find(ext);
				if (it != _types.end())
					return it->second;
				return default_type;
			}

			std::string DefaultErrorPages::get_default_content(uint16_t status) {
				std::ostringstream oss;
				const std::string& msg = StatusRegistry::get_phrase(static_cast<HttpStatus>(status));
				oss << "<html>\r\n"
					<< "<head><title>" << status << " " << msg << "</title></head>\r\n"
					<< "<body>\r\n"
					<< "<center><h1>" << status << " " << msg << "</h1></center>\r\n";
				return oss.str();
			}

		}
	}
}
