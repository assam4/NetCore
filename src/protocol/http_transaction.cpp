#include "virtualhost.hpp"
#include "http_request.hpp"
#include "http_transaction.hpp"

namespace http {
	namespace core  {

		static std::string strip_location_prefix(const std::string& location_path, const std::string& uri) {
			if (location_path == "/")
				return uri;
			if (uri == location_path)
				return "/";
			std::string stripped = uri.substr(location_path.length());
			if (stripped.empty())
				return "/";
			return (stripped[0] == '/') ? stripped : "/" + stripped;
		}

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
					bool boundary = loc_path == "/" || uri_path.length() == loc_path.length() || uri_path[loc_path.length()] == '/';
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

	types::HttpStatus HttpTransaction::validate_location_rules(const Request& req, const types::__location& loc) {
		if (req.body.content.length() > loc.content.client_max_body_size)
			return types::PAYLOAD_TOO_LARGE;
		if (!(loc.content.allowed_methods & req.start_line.method))
			return types::METHOD_NOT_ALLOWED;
		std::string full_path = loc.content.root + strip_location_prefix(loc.route.path, req.start_line.uri);
		if (access(full_path.c_str(), F_OK) != 0)
			return types::NOT_FOUND;
		if (access(full_path.c_str(), R_OK) != 0)
			return types::FORBIDDEN;
		return types::OK;
	}
	}
}