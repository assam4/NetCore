#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "http_upload.hpp"
#include "utils.hpp"

namespace http {
	namespace core {

		std::string Upload::trim_copy(const std::string& value) {
            size_t  begin = 0, end = value.size();
            for (; begin < end;)
                if (std::isspace(static_cast<unsigned char>(value[begin])) && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
                    ++begin;
                    --end;
                }
                else if (std::isspace(static_cast<unsigned char>(value[begin])))
                    ++begin;
                else if (std::isspace(static_cast<unsigned char>(value[end - 1])))
                    --end;
                else
                    break ;
			return value.substr(begin, end - begin);
		}

		std::string Upload::sanitize_filename(const std::string& input) {
			std::string filtered;
			for (size_t i = 0; i < input.size(); ++i)
				if (std::isalnum(static_cast<unsigned char>(input[i])) || input[i] == '-' || input[i] == '_' || input[i] == '.')
                    filtered += input[i];
            return filtered.empty() ? "upload.bin" : filtered;
		}

		std::string Upload::extract_header_token_value(const std::string& header, const std::string& key) {
			std::string	marker = key + "=";
			size_t pos = header.find(marker);
			if (pos == std::string::npos)
                return "";
			pos += marker.size();
			if (pos >= header.size())
				return "";
			if (header[pos] == '"') {
				++pos;
				size_t end = header.find('"', pos);
				if (end == std::string::npos)
					return "";
				return header.substr(pos, end - pos);
			}
			size_t end = header.find(';', pos);
			if (end == std::string::npos)
				end = header.size();
			return trim_copy(header.substr(pos, end - pos));
		}

		bool Upload::parse_multipart_upload(const std::string& body, const std::string& content_type, std::string& out_filename, std::string& out_payload) {
			std::string boundary;
			if (!extract_boundary(content_type, boundary))
				return false;
			std::string part_headers;
			size_t data_start = 0;
			if (!extract_first_part_headers(body, boundary, part_headers, data_start))
				return false;
			std::string cd_line;
			if (!extract_content_disposition_line(part_headers, cd_line))
				return false;
			std::string filename = extract_header_token_value(cd_line, "filename");
			if (filename.empty())
				filename = generate_fallback_filename();
			if (!extract_part_payload(body, boundary, data_start, out_payload))
				return false;
			out_filename = sanitize_filename(filename);
			return true;
		}

		bool Upload::extract_boundary(const std::string& content_type, std::string& out_boundary) {
			const std::string boundary_str = "boundary=";
			size_t bpos = content_type.find(boundary_str);
			if (bpos == std::string::npos)
				return false;
			out_boundary = trim_copy(content_type.substr(bpos + boundary_str.length()));
			if (!out_boundary.empty() && out_boundary[0] == '"' && out_boundary[out_boundary.size() - 1] == '"' && out_boundary.size() >= 2)
				out_boundary = out_boundary.substr(1, out_boundary.size() - 2);
			return !out_boundary.empty();
		}

		bool Upload::extract_first_part_headers(const std::string& body, const std::string& boundary, std::string& out_headers, size_t& out_data_start) {
			const std::string delimiter = "--" + boundary;
			size_t first = body.find(delimiter);
			if (first == std::string::npos)
				return false;
			size_t headers_start = body.find("\r\n", first);
			if (headers_start == std::string::npos)
				return false;
			headers_start += 2;
			size_t headers_end = body.find("\r\n\r\n", headers_start);
			if (headers_end == std::string::npos)
				return false;
			out_headers = body.substr(headers_start, headers_end - headers_start);
			out_data_start = headers_end + 4;
			return true;
		}

		bool Upload::extract_content_disposition_line(std::string part_headers, std::string& out_cd_line) {
			std::string part_headers_lower = part_headers;
			std::transform(part_headers_lower.begin(), part_headers_lower.end(), part_headers_lower.begin(), static_cast<int(*)(int)>(std::tolower));
			size_t cd_pos = part_headers_lower.find("content-disposition:");
			if (cd_pos == std::string::npos)
				return false;
			size_t cd_end = part_headers.find("\r\n", cd_pos);
			out_cd_line = part_headers.substr(cd_pos, cd_end == std::string::npos ? std::string::npos : cd_end - cd_pos);
			return true;
		}

		bool Upload::extract_part_payload(const std::string& body, const std::string& boundary, size_t data_start, std::string& out_payload) {
			const std::string delimiter = "--" + boundary;
			size_t next_boundary = body.find("\r\n" + delimiter, data_start);
			if (next_boundary == std::string::npos)
				return false;
			out_payload = body.substr(data_start, next_boundary - data_start);
			return true;
		}

		std::string Upload::build_upload_target(const std::string& upload_dir, const std::string& filename) {
			if (upload_dir.empty())
				return "";
			std::string target = upload_dir;
			if (target[target.size() - 1] != '/')
				target += '/';
			target += filename;
			return target;
		}

		std::string Upload::build_upload_uri(const std::string& req_uri, const std::string& filename) {
			std::string uri = req_uri;
			if (uri.empty())
				uri = "/";
			if (uri[uri.size() - 1] != '/')
				uri += '/';
			uri += filename;
			return uri;
		}

		bool    Upload::ensure_directory(const std::string& path) {
			if (path.empty() || path[0] != '/')
				return false;
			if (path == "/")
				return true;
            size_t  count = 0;
			for (size_t i = 0; i < path.size(); ++i) {
                ++count;
				if ((path[i] != '/' && i + 1 != path.size()) || (count == 1 && path[0] == '/'))
					continue;
				if (::mkdir(path.substr(0, count).c_str(), 0755) != 0 && errno != EEXIST)
					return false;
			}
			return true;
		}

		std::string Upload::generate_fallback_filename() {
			std::ostringstream oss;
			oss << "upload_" << static_cast<unsigned long>(std::time(NULL)) << "_" << static_cast<unsigned long>(::getpid()) << ".bin";
			return oss.str();
		}

		bool Upload::read_content_type(const Request& req, std::string& out_content_type) {
			std::map<std::string, std::vector<std::string> >::const_iterator ct = req.headers.header_map.find("content-type");
			if (ct == req.headers.header_map.end() || ct->second.empty())
				return false;
			out_content_type = ct->second.front();
			return true;
		}

		bool Upload::resolve_payload_and_filename(const Request& req, std::string& io_payload, std::string& io_filename, types::HttpStatus& out_status, std::string& out_body) {
			std::string content_type;
			if (!read_content_type(req, content_type))
				return true;
			if (content_type.find("application/octet-stream") != std::string::npos) {
				std::map<std::string, std::vector<std::string> >::const_iterator cd = req.headers.header_map.find("content-disposition");
				if (cd != req.headers.header_map.end() && !cd->second.empty()) {
					const std::string header_filename = extract_header_token_value(cd->second.front(), "filename");
					if (!header_filename.empty())
						io_filename = sanitize_filename(header_filename);
				}
				return true;
			}
			if (content_type.find("multipart/form-data") == std::string::npos)
				return true;
			std::string multipart_filename;
			std::string multipart_payload;
			if (!parse_multipart_upload(io_payload, content_type, multipart_filename, multipart_payload)) {
				out_status = types::BAD_REQUEST;
				out_body = "Invalid multipart upload payload";
				return false;
			}
			io_filename = multipart_filename;
			io_payload = multipart_payload;
			return true;
		}

		bool Upload::write_payload_to_target(const std::string& target, const std::string& payload, types::HttpStatus& out_status, std::string& out_body) {
			std::ofstream out(target.c_str(), std::ios::binary | std::ios::trunc);
			if (!out.is_open()) {
				out_status = types::INTERNAL_SERVER_ERROR;
				out_body = "Failed to open upload target";
				return false;
			}
			out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
			out.close();
			if (!out) {
				out_status = types::INTERNAL_SERVER_ERROR;
				out_body = "Failed to write upload file";
				return false;
			}
			return true;
		}

		void    Upload::set_created_response(const Request& req, const std::string& filename, types::HttpStatus& out_status, std::map<std::string, std::string>& out_headers, std::string& out_body) {
			const std::string saved_uri = build_upload_uri(req.start_line.uri, filename);
			out_status = types::CREATED;
			out_headers["Location"] = saved_uri;
			out_headers["Content-Location"] = saved_uri;
			out_body = "Created";
		}

		bool Upload::handle_request(const Request& req, const types::__location& location, types::HttpStatus& out_status, std::map<std::string, std::string>& out_headers, std::string& out_body) {
			if (req.start_line.method != types::POST || location.upload_location.empty())
				return false;
			if (!ensure_directory(location.upload_location)) {
				out_status = types::INTERNAL_SERVER_ERROR;
				out_body = "Failed to prepare upload directory";
				return true;
			}
			std::string payload = req.body.content, filename = generate_fallback_filename();
			if (!resolve_payload_and_filename(req, payload, filename, out_status, out_body))
				return true;
			std::string target = build_upload_target(location.upload_location, filename);
			if (!write_payload_to_target(target, payload, out_status, out_body))
				return true;
			set_created_response(req, filename, out_status, out_headers, out_body);
			return true;
		}

	}
}
