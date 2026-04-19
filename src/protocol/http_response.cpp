#include "http_response.hpp"
#include <fstream>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include "utils.hpp"
#include "http_types.hpp"
#include "CGI.hpp"
#include "http_upload.hpp"

namespace http {
	namespace core {

		bool Response::check_conditional(const Request& req, const Response::_http_response& res) {
			const std::map<std::string, std::vector<std::string> >& header_map = req.headers.header_map;
			std::map<std::string, std::vector<std::string> >::const_iterator inm = header_map.find("if-none-match");
			if (inm != header_map.end()) {
				std::map<std::string, std::string>::const_iterator etag_it = res._headers.find("ETag");
				if (etag_it != res._headers.end()) {
					for (size_t i = 0; i < inm->second.size(); ++i) {
						if (inm->second[i] == "*" ||
							inm->second[i] == etag_it->second)
							return true;
					}
				}
			}
			std::map<std::string, std::vector<std::string> >::const_iterator ims = header_map.find("if-modified-since");
			if (ims != header_map.end() && !ims->second.empty()) {
				std::map<std::string, std::string>::const_iterator lm_it = res._headers.find("Last-Modified");
				if (lm_it != res._headers.end()) {
					if (lm_it->second <= ims->second.front())
						return true;
				}
			}
			return false;
		}

		bool Response::check_precondition(const Request& req, const Response::_http_response& res) {
			const std::map<std::string, std::vector<std::string> >& header_map = req.headers.header_map;
			std::map<std::string, std::vector<std::string> >::const_iterator im =
			header_map.find("if-match");
			if (im != header_map.end()) {
			std::map<std::string, std::string>::const_iterator etag_it = res._headers.find("ETag");
			bool matched = false;
			for (size_t i = 0; i < im->second.size(); ++i) {
				if (im->second[i] == "*" ||
					(etag_it != res._headers.end() &&
					 im->second[i] == etag_it->second)) {
					matched = true;
					break;
				}
			}
			if (!matched)
				return true;
			}
			std::map<std::string, std::vector<std::string> >::const_iterator ius = header_map.find("if-unmodified-since");
			if (ius != header_map.end() && !ius->second.empty()) {
				std::map<std::string, std::string>::const_iterator lm_it = res._headers.find("Last-Modified");
				if (lm_it != res._headers.end()) {
					if (lm_it->second > ius->second.front())
						return true;
				}
			}
			return false;
		}

		std::string Response::uri_encode(const std::string& uri) {
			std::string encoded;
			static const char hex[] = "0123456789abcdef";
			for (size_t i = 0; i < uri.size(); ++i) {
				unsigned char c = static_cast<unsigned char>(uri[i]);
				if (std::isalnum(c) || c == '-' || c == '_' ||
						c == '.' || c == '~' || c == '/')
					encoded += c;
				else {
					encoded += '%';
					encoded += hex[c >> 4];
					encoded += hex[c & 0xF];
				}
			}
			return encoded;
		}

		std::string Response::read_file(const std::string& path) {
			std::ifstream file(path.c_str(), std::ios::binary);
			if (!file.is_open())
				return "";
			std::stringstream ss;
			ss << file.rdbuf();
			return ss.str();
		}

		std::string Response::resolve_path(const std::string& root, const std::string& uri) {
			std::string path = root;
			if (!path.empty() && path[path.size() - 1] == '/')
				path.erase(path.size() - 1);
			if (uri.empty() || uri[0] != '/')
				path += '/';
			path += uri;
			return path;
		}

		std::string Response::find_index(const std::string& dir_path, const std::set<std::string>& index_files) {
			for (std::set<std::string>::const_iterator it = index_files.begin(); it != index_files.end(); ++it) {
				std::string full = dir_path;
				if (!full.empty() && full[full.size() - 1] != '/')
					full += '/';
				full += *it;
				struct stat st;
				if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode))
					return full;
			}
			return "";
		}

		std::string Response::build_directory_listing(const std::string& uri, const std::string& dir_path) {
			DIR* dir = opendir(dir_path.c_str());
			if (!dir)
				return "";
			std::ostringstream html;
			html << "<!DOCTYPE html>\r\n<html>\r\n<head><title>Index of "
				 << uri << "</title></head>\r\n<body>\r\n"
				 << "<h1>Index of " << uri << "</h1>\r\n<hr>\r\n<pre>\r\n";
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				std::string name = entry->d_name;
				if (name == "." || name == "..")
					continue;
				std::string full = dir_path;
				if (!full.empty() && full[full.size() - 1] != '/')
					full += '/';
				full += name;
				struct stat st;
				if (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
					name += '/';
				html << "<a href=\"" << uri_encode(name) << "\">" << name << "</a>\r\n";
			}
			closedir(dir);
			html << "</pre>\r\n<hr>\r\n</body>\r\n</html>\r\n";
			return html.str();
		}

		void Response::make_error(_http_response& res, types::HttpStatus status, const std::map<uint16_t, std::string>& error_pages, const std::string& root) {
			res._status = status;
			res._headers["Content-Type"] = "text/html";
			res._headers["Cache-Control"] = "no-store, no-cache, must-revalidate";
			res._headers["Pragma"] = "no-cache";
			res._headers["Expires"] = "0";
			std::map<uint16_t, std::string>::const_iterator it = error_pages.find(static_cast<uint16_t>(status));
			if (it != error_pages.end()) {
				std::string body = read_file(it->second);
				if (body.empty() && !root.empty() && !it->second.empty() && it->second[0] == '/')
					body = read_file(resolve_path(root, it->second));
				if (!body.empty()) {
					res._body = body;
					return;
				}
			}
			res._body = types::DefaultErrorPages::get_default_content(static_cast<uint16_t>(status));
		}

		void Response::make_redirect(_http_response& res, uint16_t code, const std::string& new_path) {
			res._status = static_cast<types::HttpStatus>(code);
			res._headers["Location"] = new_path;
			res._headers["Content-Type"] = "text/html";
			std::ostringstream ss;
			ss << "<!DOCTYPE html>\r\n<html>\r\n<body>\r\n<h1>"
			   << code << " "
			   << types::StatusRegistry::get_phrase(static_cast<types::HttpStatus>(code))
			   << "</h1>\r\n<p>Redirecting to <a href=\""
			   << new_path << "\">" << new_path
			   << "</a></p>\r\n</body>\r\n</html>\r\n";
			res._body = ss.str();
		}

		void Response::make_static(_http_response& res, const std::string& path) {
			std::string body = read_file(path);
			if (body.empty()) {
				make_error(res, types::NOT_FOUND, std::map<uint16_t, std::string>());
				return;
			}
			res._status = types::OK;
			res._body = body;
			set_content_type_field(res, path);
			set_last_modified_field(res, path);
			set_etag_field(res, path, false);
		}

		void Response::make_autoindex(_http_response& res, const std::string& uri, const std::string& dir_path) {
			std::string body = build_directory_listing(uri, dir_path);
			if (body.empty()) {
				make_error(res, types::FORBIDDEN, std::map<uint16_t, std::string>());
				return;
			}
			res._status = types::OK;
			res._body   = body;
			res._headers["Content-Type"] = "text/html";
		}

		void Response::make_cgi(_http_response& res, const Request& req, const std::string& scriptPath,
						const std::string& ext, const types::__location& location, uint16_t serverPort){
			std::map<std::string, std::string>::const_iterator it = location.cgi_extension.find(ext);
			if (it == location.cgi_extension.end()) {
				make_error(res, types::FORBIDDEN, location.content.error_pages);
				return;
			}
			const std::string& interpreterPath = it->second;
			Response::_http_response cgi_res =
				CGI::exec(req, scriptPath, interpreterPath, serverPort);
			if (cgi_res._status == types::NOT_FOUND && ext == ".php" && cgi_res._body == "CGI interpreter not found") {
				make_static(res, scriptPath);
				return;
			}
			res = cgi_res;
			res._version = "HTTP/1.1";
		}

		void Response::set_connection_field(_http_response& res, const Request& req) {
			const std::map<std::string, std::vector<std::string> >& header_map = req.headers.header_map;
			std::map<std::string, std::vector<std::string> >::const_iterator it = header_map.find("connection");
			std::string conn_value;
			if (it != header_map.end()) {
				bool has_close  = false;
				bool has_keep_alive = false;
				for (std::vector<std::string>::const_iterator vit = it->second.begin(); vit != it->second.end(); ++vit) {
					std::string token = *vit;
					std::transform(token.begin(), token.end(), token.begin(), static_cast<int(*)(int)>(std::tolower));
					if (token.find("close") != std::string::npos)
						has_close = true;
					else if (token.find("keep-alive") != std::string::npos)
						has_keep_alive = true;
				}
				if (has_close)
					conn_value = "close";
				else if (has_keep_alive && req.start_line.version == "HTTP/1.1")
					conn_value = "keep-alive";
				else
					conn_value = "close";
			} else {
				conn_value = (req.start_line.version == "HTTP/1.1") ? "keep-alive" : "close";
			}
			res._headers["Connection"] = conn_value;
		}

		void Response::set_server_field(_http_response& res) {
			res._headers["Server"] = "webserv";
		}

		void Response::set_date_field(_http_response& res) {
			std::time_t t = std::time(NULL);
			std::tm* gmt = std::gmtime(&t);
			char date[128];
			std::strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);
			res._headers["Date"] = std::string(date);
		}

		void Response::set_body_length_field(_http_response& res) {
			bool is_1_1 = (res._version == "HTTP/1.1");
			std::string connection = "close";
			std::map<std::string, std::string>::const_iterator it = res._headers.find("Connection");
			if (it != res._headers.end())
				connection = it->second;
			if (!res._body.empty())
				res._headers["Content-Length"] = to_string(res._body.size());
			else if (is_1_1 && connection == "keep-alive")
				res._headers["Transfer-Encoding"] = "chunked";
			else
				res._headers["Connection"] = "close";
		}

		void Response::set_content_type_field(_http_response& res, const std::string& path) {
			std::string ext;
			size_t pos = path.find_last_of('.');
			if (pos != std::string::npos && pos + 1 < path.size())
				ext = path.substr(pos);
			res._headers["Content-Type"] = types::MimeTypes::get_mime_type(ext);
		}

		void Response::set_allow_field(_http_response& res, uint8_t methods) {
			std::string list;
			if (methods & types::GET)
				list += "GET";
			if (methods & types::POST) {
				if (!list.empty())
					list += ", ";
				list += "POST";
			}
			if (methods & types::DEL) {
				if (!list.empty())
					list += ", ";
				list += "DELETE";
			}
			res._headers["Allow"] = list;
		}

		void Response::set_last_modified_field(_http_response& res, const std::string& path) {
			struct stat buffer;
			if (stat(path.c_str(), &buffer) == 0) {
				std::tm* gmt = std::gmtime(&buffer.st_mtime);
				char date[128];
				std::strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);
				res._headers["Last-Modified"] = std::string(date);
			}
		}

		std::string Response::compute_strong_etag(const std::string& path) {
			std::string body = read_file(path);
			if (body.empty())
				return "";
			// FNV-1a 32-bit
			uint32_t hash = 2166136261u;
			for (size_t i = 0; i < body.size(); ++i) {
				hash ^= static_cast<unsigned char>(body[i]);
				hash *= 16777619u;
			}
			std::ostringstream oss;
			oss << "\"" << std::hex << hash << "\"";
			return oss.str();
		}

		std::string Response::compute_weak_etag(const std::string& path) {
			struct stat buffer;
			if (stat(path.c_str(), &buffer) < 0)
				return "";
			std::ostringstream oss;
			oss << "W/\"" << buffer.st_mtime << "-" << buffer.st_size << "\"";
			return oss.str();
		}

		void Response::set_etag_field(_http_response& res, const std::string& path, bool use_strong) {
			std::string etag = use_strong ? compute_strong_etag(path) : compute_weak_etag(path);
			if (!etag.empty())
				res._headers["ETag"] = etag;
		}

		void Response::set_common_fields(_http_response& res, const Request& req) {
			set_server_field(res);
			set_date_field(res);
			set_connection_field(res, req);
			set_body_length_field(res);
		}

		Response::_http_response Response::make_response(const std::pair<types::HttpStatus, Request>& status_req, const types::__location& location, uint16_t server_port) {
			_http_response res;
			res._version = "HTTP/1.1";
			res._status = types::OK;
			const types::HttpStatus parse_status = status_req.first;
			const Request& req = status_req.second;

			if (parse_status != types::OK) {
				make_error(res, parse_status, location.content.error_pages, location.content.root);
				set_common_fields(res, req);
				return res;
			}
			if (!(location.content.allowed_methods & static_cast<uint8_t>(req.start_line.method))) {
				std::cerr << "[DEBUG] Method check FAILED: location=" << location.route.path
					<< " allowed=" << (int)location.content.allowed_methods
					<< " method=" << (int)req.start_line.method << std::endl;
				make_error(res, types::METHOD_NOT_ALLOWED, location.content.error_pages, location.content.root);
				set_allow_field(res, location.content.allowed_methods);
				set_common_fields(res, req);
				return res;
			}
			if (location.route.code != 0) {
				make_redirect(res, location.route.code, location.route.new_path);
				set_common_fields(res, req);
				return res;
			}

			types::HttpStatus upload_status = types::OK;
			std::string upload_body;
			if (Upload::handle_request(req, location, upload_status, res._headers, upload_body)) {
				res._status = upload_status;
				res._body = upload_body;
				set_common_fields(res, req);
				return res;
			}
			if (req.start_line.method == types::POST && location.upload_location.empty()) {
				make_error(res, types::NOT_IMPLEMENTED, location.content.error_pages, location.content.root);
				set_common_fields(res, req);
				return res;
			}

			if (req.start_line.method == types::DEL) {
				std::string del_path = resolve_path(location.content.root, req.start_line.uri);
				struct stat del_st;
				if (stat(del_path.c_str(), &del_st) != 0) {
					make_error(res, types::NOT_FOUND, location.content.error_pages, location.content.root);
					set_common_fields(res, req);
					return res;
				}
				if (S_ISDIR(del_st.st_mode)) {
					make_error(res, types::FORBIDDEN, location.content.error_pages, location.content.root);
					set_common_fields(res, req);
					return res;
				}
				if (std::remove(del_path.c_str()) != 0) {
					make_error(res, types::FORBIDDEN, location.content.error_pages, location.content.root);
					set_common_fields(res, req);
					return res;
				}
				res._status = types::NO_CONTENT;  // 204
				res._body.clear();
				set_common_fields(res, req);
				return res;
			}

			std::string fs_path = resolve_path(location.content.root, req.start_line.uri);
			struct stat st;
			if (stat(fs_path.c_str(), &st) != 0) {
				make_error(res, types::NOT_FOUND, location.content.error_pages, location.content.root);
				set_server_field(res);
				set_date_field(res);
				set_connection_field(res, req);
				set_body_length_field(res);
				return res;
			}

			// ── 5. Directory ──────────────────────────────────────────────────────
			if (S_ISDIR(st.st_mode)) {
				const std::string& uri = req.start_line.uri;
				if (uri.empty() || uri[uri.size() - 1] != '/') {
					make_redirect(res, 301, uri + "/");
					set_common_fields(res, req);
					return res;
				}
				std::string index_path = find_index(fs_path, location.content.index);
				if (!index_path.empty()) {
					fs_path = index_path;
					if (stat(fs_path.c_str(), &st) != 0) {
						make_error(res, types::NOT_FOUND, location.content.error_pages, location.content.root);
						set_server_field(res);
						set_date_field(res);
						set_connection_field(res, req);
						set_body_length_field(res);
						return res;
					}
				} else if (location.content.autoindex) {
					make_autoindex(res, req.start_line.uri, fs_path);
					set_server_field(res);
					set_date_field(res);
					set_connection_field(res, req);
					set_body_length_field(res);
					return res;
				} else {
					make_error(res, types::FORBIDDEN, location.content.error_pages, location.content.root);
					set_server_field(res);
					set_date_field(res);
					set_connection_field(res, req);
					set_body_length_field(res);
					return res;
				}
			}

			// ── 6. CGI ────────────────────────────────────────────────────────────
			if (!location.cgi_extension.empty()) {
				size_t dot = fs_path.find_last_of('.');
				if (dot != std::string::npos) {
					std::string ext = fs_path.substr(dot);
					if (location.cgi_extension.find(ext) != location.cgi_extension.end()) {
						make_cgi(res, req, fs_path, ext, location, server_port);
						set_common_fields(res, req);
						return res;
					}
				}
			}

			// ── 7. Static file ────────────────────────────────────────────────────
			// make_static sets ETag + Last-Modified — conditional checks must come after
			make_static(res, fs_path);

			// ── 8. Conditional request checks (RFC 7232) ──────────────────────────
			// RFC 7232 §6: evaluate If-Match / If-Unmodified-Since BEFORE
			//              If-None-Match / If-Modified-Since
			if (check_precondition(req, res)) {
				// If-Match failed or If-Unmodified-Since failed → 412
				make_error(res, types::PRECONDITION_FAILED, location.content.error_pages, location.content.root);
			} else if (req.start_line.method == types::GET &&
					   check_conditional(req, res)) {
				// If-None-Match or If-Modified-Since matched → 304
				// RFC 7232 §4.1: keep ETag + Last-Modified, drop body + content headers
				res._status = types::NOT_MODIFIED;
				res._body.clear();
				res._headers.erase("Content-Type");
				res._headers.erase("Content-Length");
				res._headers.erase("Transfer-Encoding");
			}

			set_server_field(res);
			set_date_field(res);
			set_connection_field(res, req);
			set_body_length_field(res);
			return res;
		}

		std::string Response::serialize(const _http_response& response) {
			std::ostringstream out;
			out << response._version << " "
				<< static_cast<int>(response._status) << " "
				<< types::StatusRegistry::get_phrase(response._status)
				<< "\r\n";
			for (std::map<std::string, std::string>::const_iterator it = response._headers.begin(); it != response._headers.end(); ++it)
				out << it->first << ": " << it->second << "\r\n";
			for (std::vector<Cookie>::const_iterator it = response._cookies.begin(); it != response._cookies.end(); ++it)
				out << "Set-Cookie: " << it->serialize_set_cookie() << "\r\n";
			out << "\r\n";
			if (!response._body.empty())
				out << response._body;
			return out.str();
		}
	}
}
