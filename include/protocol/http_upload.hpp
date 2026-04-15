#ifndef HTTP_UPLOAD_HPP
# define HTTP_UPLOAD_HPP

# include <string>
# include <map>
# include "http_request.hpp"
# include "server_types.hpp"
# include "http_types.hpp"

namespace http {
	namespace core {

		/**
		 * @brief Handles file upload requests for configured upload locations.
		 *
		 * Supports both multipart/form-data payloads and raw
		 * application/octet-stream uploads.
		 */
		class   Upload {
            public:
				/**
				 * @brief Processes POST upload request and writes payload to disk.
				 *
				 * @param req Parsed HTTP request.
				 * @param location Matched location configuration.
				 * @param out_status Output response status.
				 * @param out_headers Output response headers.
				 * @param out_body Output response body.
				 * @return true when this component handled the request.
				 * @return false when request is not an upload candidate.
				 */
				static bool handle_request(const Request&, const types::__location&, types::HttpStatus&, std::map<std::string, std::string>&, std::string&);
			private:
				Upload();
				Upload(const Upload&);
				Upload& operator=(const Upload&);

				static std::string trim_copy(const std::string&);
				static std::string sanitize_filename(const std::string&);
				static std::string extract_header_token_value(const std::string&, const std::string&);
				static bool parse_multipart_upload(const std::string&, const std::string&, std::string&, std::string&);
				static bool extract_boundary(const std::string&, std::string&);
				static bool extract_first_part_headers(const std::string&, const std::string&, std::string&, size_t&);
				static bool extract_content_disposition_line(std::string, std::string&);
				static bool extract_part_payload(const std::string&, const std::string&, size_t, std::string&);
				static std::string build_upload_target(const std::string&, const std::string&);
				static std::string build_upload_uri(const std::string&, const std::string&);
				static bool ensure_directory(const std::string&);
				static std::string generate_fallback_filename();
				static bool read_content_type(const Request&, std::string&);
				static bool resolve_payload_and_filename(const Request&, std::string&, std::string&, types::HttpStatus&, std::string&);
				static bool write_payload_to_target(const std::string&, const std::string&, types::HttpStatus&, std::string&);
				static void set_created_response(const Request&, const std::string&, types::HttpStatus&, std::map<std::string, std::string>&, std::string&);
        };
	}
}

#endif
