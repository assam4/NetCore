#ifndef HTTP_RESPONSE
# define HTTP_RESPONSE

#include <map>
#include "http_types.hpp"
#include "http_request.hpp"
#include <string>
#include <stdint.h>

namespace http {
	namespace core {

		/**
		 * @class Response
		 * @brief Response builder and serializer for HTTP replies.
		 * @details Converts parsed request/result pairs into wire-ready responses.
		 *          Populates status line, headers, and body/error page payload.
		 */
		class Response {
			public:
				/**
				 * @struct _http_response
				 * @brief Internal aggregate representing a complete HTTP response.
				 * @details Stores protocol version, status, headers, and body text.
				 *          Used as intermediate form before serialization.
				 */
				struct _http_response {
					std::string _version;
					types::HttpStatus _status;
					std::map<std::string, std::string> _headers;
					std::string _body;
				};
				static _http_response make_response(const std::pair<types::HttpStatus, Request::__http_request>& req);
				static std::string serialize(const _http_response& response);
			private:
				Response();
				Response(const Response&);
				Response& operator=(const Response&);

				static std::string read_file(const std::string& path);
				static void set_connection_field(_http_response& res, const Request::__http_request& req);
				static void set_server_field(_http_response& res);
				static void set_date_field(_http_response& res);
				static void set_body_length_field(_http_response& res);
				static void set_content_type_field(Response::_http_response& res, const std::string& path);
				static void set_allow_field(Response::_http_response& res, uint8_t methods);
				static void set_last_modified_field(Response::_http_response& res, const std::string& path);
				static void set_etag_field(Response::_http_response& res,const std::string& path, bool use_strong);
				static std::string compute_weak_etag(const std::string& path);
				static std::string compute_strong_etag(const std::string& path);
				static void make_error(_http_response& res, types::HttpStatus status);

		};
	}
}

#endif
