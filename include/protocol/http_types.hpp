#ifndef HTTP_TYPES_HPP
# define HTTP_TYPES_HPP

# include <stdint.h>
# include <map>
# include <string>

namespace http {
	namespace core {
		namespace types {

			enum HttpStatus {
				// 1xx - Informational
				CONTINUE = 100,
				SWITCHING_PROTOCOLS = 101,
				PROCESSING = 102,
				EARLY_HINTS = 103,
				// 2xx - Success
				OK = 200,
				CREATED = 201,
				ACCEPTED = 202,
				NON_AUTHORITATIVE_INFORMATION = 203,
				NO_CONTENT = 204,
				RESET_CONTENT = 205,
				PARTIAL_CONTENT = 206,
				MULTI_STATUS = 207,
				ALREADY_REPORTED = 208,
				IM_USED = 226,
				// 3xx - Redirection
				MULTIPLE_CHOICES = 300,
				MOVED_PERMANENTLY = 301,
				FOUND = 302,
				SEE_OTHER = 303,
				NOT_MODIFIED = 304,
				USE_PROXY = 305,
				UNUSED = 306,
				TEMPORARY_REDIRECT = 307,
				PERMANENT_REDIRECT = 308,
				// 4xx - Client Errors
				BAD_REQUEST = 400,
				UNAUTHORIZED = 401,
				PAYMENT_REQUIRED = 402,
				FORBIDDEN = 403,
				NOT_FOUND = 404,
				METHOD_NOT_ALLOWED = 405,
				NOT_ACCEPTABLE = 406,
				PROXY_AUTHENTICATION_REQUIRED = 407,
				REQUEST_TIMEOUT = 408,
				CONFLICT = 409,
				GONE = 410,
				LENGTH_REQUIRED = 411,
				PRECONDITION_FAILED = 412,
				PAYLOAD_TOO_LARGE = 413,
				URI_TOO_LONG = 414,
				UNSUPPORTED_MEDIA_TYPE = 415,
				RANGE_NOT_SATISFIABLE = 416,
				EXPECTATION_FAILED = 417,
				I_AM_NOT_TEAPOT = 418,
				MISDIRECTED_REQUEST = 421,
				UNPROCESSABLE_CONTENT = 422,
				LOCKED = 423,
				FAILED_DEPENDENCY = 424,
				TOO_EARLY = 425,
				UPGRADE_REQUIRED = 426,
				PRECONDITION_REQUIRED = 428,
				TOO_MANY_REQUESTS = 429,
				REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
				UNAVAILABLE_FOR_LEGAL_REASONS = 451,
				// 5xx - Server Errors
				INTERNAL_SERVER_ERROR = 500,
				NOT_IMPLEMENTED = 501,
				BAD_GATEWAY = 502,
				SERVICE_UNAVAILABLE = 503,
				GATEWAY_TIMEOUT = 504,
				HTTP_VERSION_NOT_SUPPORTED = 505,
				VARIANT_ALSO_NEGOTIATES = 506,
				INSUFFICIENT_STORAGE = 507,
				LOOP_DETECTED = 508,
				NOT_EXTENDED = 510,
				NETWORK_AUTHENTICATION_REQUIRED = 511
			};

			enum HttpMethod {
				GET = 1 << 0,
				POST = 1 << 1,
				DEL = 1 << 2
			};

			/**
			 * @struct HttpError
			 * @brief Lightweight exception that carries HTTP status context.
			 * @details Wraps status code and optional message for parser/runtime errors.
			 *          Allows throwing structured HTTP failures inside core logic.
			 */
			struct HttpError : std::exception {
				types::HttpStatus code;
				std::string message;

				HttpError(types::HttpStatus c, const std::string& m) : code(c), message(m) {}
				virtual ~HttpError() throw() {}
				virtual const char* what() const throw() {
					return message.empty() ? "Invalid request" : message.c_str();
				}
			};

			/**
			 * @class StatusRegistry
			 * @brief Static lookup for HTTP status reason phrases.
			 * @details Maps status codes to standard textual descriptions.
			 *          Used while building response start lines and error pages.
			 */
			class StatusRegistry {
				private:
					static const std::map<HttpStatus, std::string> _phrases;

					StatusRegistry();
					StatusRegistry(const StatusRegistry&);
					StatusRegistry& operator=(const StatusRegistry&);
					static std::map<HttpStatus, std::string> init();
				public:
					static const std::string& get_phrase(types::HttpStatus status);
			};

			/**
			 * @class MimeTypes
			 * @brief Registry of file extension to MIME type mappings.
			 * @details Loads mappings from configuration and serves content-type lookups.
			 *          Used by response generation for static file delivery.
			 */
			class MimeTypes {
				private:
					static const std::map<std::string, std::string> _types;

					MimeTypes();
					MimeTypes(const MimeTypes&);
					MimeTypes& operator=(const MimeTypes&);
					static std::map<std::string, std::string> init();
				public:

					static const std::string& get_mime_type(const std::string& type);
			};

			/**
			 * @class DefaultErrorPages
			 * @brief Built-in fallback HTML bodies for common HTTP errors.
			 * @details Returns a default response page when a configured error page is missing.
			 *          Keeps error responses stable even without custom filesystem templates.
			 */
			class DefaultErrorPages {
				private:
					DefaultErrorPages();
					DefaultErrorPages(const DefaultErrorPages&);
					DefaultErrorPages& operator=(const DefaultErrorPages&);
				public:
					static std::string get_default_content(uint16_t status);
			};
		}
	}
}

#endif
