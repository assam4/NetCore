#include "http_response.hpp"

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
	}
}
