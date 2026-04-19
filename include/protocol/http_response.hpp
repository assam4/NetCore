#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <map>
# include <string>
# include <vector>
# include <sys/stat.h>
# include <stdint.h>
# include "http_types.hpp"
# include "http_cookie.hpp"
# include "http_request.hpp"
# include "server_types.hpp"

namespace http {
	namespace core {

		/**
		 * @class Response
		 * @brief Response builder and serializer for HTTP replies.
		 * @details Converts a parsed Request and a matched location config
		 *          into a wire-ready HTTP response string.
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
				std::vector<Cookie> _cookies;
				std::string _body;
			};

			static _http_response make_response(const std::pair<types::HttpStatus, Request>&, const types::__location&, uint16_t);
			static std::string serialize(const _http_response&);
		private:
			Response();
			Response(const Response&);
			Response& operator=(const Response&);

			static bool check_conditional(const Request& req, const Response::_http_response& res);
			static bool check_precondition(const Request& req, const Response::_http_response& res);
			static std::string uri_encode(const std::string& uri);
			static std::string read_file(const std::string& path);
			static std::string resolve_path(const std::string& root, const std::string& uri);
			static std::string find_index(const std::string& dir_path, const std::set<std::string>& index_files);
			static std::string build_directory_listing(const std::string& uri, const std::string& dir_path);

			static void make_error(_http_response& res, types::HttpStatus status, const std::map<uint16_t, std::string>& error_pages, const std::string& root = "");
			static void make_redirect(_http_response& res, uint16_t code, const std::string& new_path);
			static void make_static(_http_response& res, const std::string& path);
			static void make_cgi(_http_response& res, const Request& req, const std::string& path, const std::string& ext, const types::__location& location, uint16_t server_port);
			static void make_autoindex(_http_response& res, const std::string& uri, const std::string& dir_path);

			static void set_connection_field(_http_response& res, const Request& req);
			static void set_server_field(_http_response& res);
			static void set_date_field(_http_response& res);
			static void set_body_length_field(_http_response& res);
			static void set_content_type_field(_http_response& res, const std::string& path);
			static void set_allow_field(_http_response& res, uint8_t methods);
			static void set_last_modified_field(_http_response& res, const std::string& path);
			static void set_etag_field(_http_response& res, const std::string& path, bool use_strong);
			static void set_common_fields(_http_response& res, const Request& req);

			static std::string compute_strong_etag(const std::string& path);
			static std::string compute_weak_etag(const std::string& path);

			static void init_response(_http_response& res, const Request& req);
			static bool handle_parse_error(_http_response& res, const Request& req, types::HttpStatus parse_status, const types::__location& location);
			static bool handle_method_check(_http_response& res, const Request& req, const types::__location& location);
			static bool handle_redirect(_http_response& res, const Request& req, const types::__location& location);
			static bool handle_upload(_http_response& res, const Request& req, const types::__location& location);
			static bool handle_delete(_http_response& res, const Request& req, const types::__location& location);
			static bool handle_stat(_http_response& res, const Request& req, const types::__location& location, const std::string& fs_path, struct stat& st);
			static bool handle_cgi(_http_response& res, const Request& req, const types::__location& location, const std::string& fs_path, uint16_t server_port);
			static bool handle_directory(_http_response& res, const Request& req, const types::__location& location, std::string& fs_path, struct stat& st);
			static void handle_static(_http_response& res, const Request& req, const types::__location& location, const std::string& fs_path);
		};
	}
}

#endif
