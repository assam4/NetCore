#ifndef HTTP_RESPONSE
# define HTTP_RESPONSE

#include <map>
#include "http_types.hpp"
#include "http_request.hpp"
#include <string>
#include <stdint.h>

namespace http {
	namespace core {

		template<
		std::string to_string(int)

		class StatusRegistry {
			private:
				std::map<types::HttpStatus, std::string> _phrases;

				StatusRegistry();

				StatusRegistry(const StatusRegistry&);
				StatusRegistry& operator=(const StatusRegistry&);
			public:
				static StatusRegistry& instance();
				std::string get_phrase(types::HttpStatus status) const;
		};

		class Response {
			public:
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
				static void make_error(_http_response& res, types::HttpStatus status);

		};
	}
}

#endif
