#include "virtualhost.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_cookie.hpp"
#include "http_transaction.hpp"

namespace http {
	namespace core  {

		const types::__location& HttpTransaction::get_best_location(const VirtualHost& vhost, const std::string& uri_path) {
			const std::vector<types::__location>& locations = vhost.get_locations();
			const types::__location* longest_prefix = NULL;
			size_t longest_len = 0;
			for (std::vector<types::__location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
				const std::string& loc_path = it->route.path;
				if (it->route.modifier == "=") {
					if (uri_path == loc_path)
						return *it;
				} else {
					bool prefix = uri_path.compare(0, loc_path.length(), loc_path) == 0;
					bool boundary = loc_path == "/" || uri_path.length() == loc_path.length() || (uri_path.length() > loc_path.length() && uri_path[loc_path.length()] == '/');
					if (prefix && boundary && loc_path.length() > longest_len) {
						longest_len = loc_path.length();
						longest_prefix = &(*it);
					}
				}
			}
			if (longest_prefix)
				return *longest_prefix;
			for (std::vector<types::__location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
				if (it->route.path == "/")
					return *it;
			}
			throw std::runtime_error("No root location found");
		}

		bool HttpTransaction::process(Connection *_conn, HttpServer& _http_server) {
			std::pair<types::HttpStatus, Request> status_req = Request::parse_message(*_conn);
			const Request& req = status_req.second;
			std::string host_header;
			std::map<std::string, std::vector<std::string> >::const_iterator host_it = req.headers.header_map.find("host");
			if (host_it != req.headers.header_map.end() && !host_it->second.empty())
				host_header = host_it->second.front();
			size_t colon_pos = host_header.find(':');
			if (colon_pos != std::string::npos)
				host_header.erase(colon_pos);
			const http::core::VirtualHost* vhost = _http_server.find_vhost(_conn->get_local_port(), host_header);
			if (!vhost)
				return false;
			const types::__location& location = HttpTransaction::get_best_location(*vhost, req.start_line.uri);
			if (status_req.first == types::OK) {
				try {
					std::map<std::string, std::vector<std::string> >::const_iterator body_mode = status_req.second.check_mandatory_headers();
					status_req.second.read_body(*_conn, body_mode, location.content.client_max_body_size);
				} catch (types::HttpStatus status) {
					status_req.first = status;
				}
			}
			Response::_http_response response = Response::make_response(status_req, location, _conn->get_local_port());
			bool created_session = false;
			std::string sid = _http_server.sessions().ensure_session(req.headers.cookies, created_session);
			if (created_session) {
				Cookie session_cookie;
				session_cookie.set_session(sid, SESSION_MAX_AGE);
				response._cookies.push_back(session_cookie);
			}
			_conn->append_write(Response::serialize(response));
			return true;
		}
	}
}
