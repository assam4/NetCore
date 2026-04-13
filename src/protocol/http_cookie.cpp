#include "http_cookie.hpp"
# include <sstream>

namespace http {
	namespace core {

		bool Cookie::fill_from_request_map(const std::map<std::string, std::string>& request_cookies, const std::string& key) {
			std::map<std::string, std::string>::const_iterator it = request_cookies.find(key);
			if (it == request_cookies.end())
				return false;
			*this = Cookie();
			name = key;
			value = it->second;
			return true;
		}

		void Cookie::set_session(const std::string& sid, long max_age_seconds) {
			name = "session_id";
			value = sid;
			path = "/";
			http_only = true;
			same_site = "Lax";
			has_max_age = true;
			max_age = max_age_seconds;
		}

		std::string Cookie::serialize_set_cookie() const {
			std::ostringstream ss;
			ss << name << "=" << value;
			if (!path.empty())      ss << "; Path=" << path;
			if (!domain.empty())    ss << "; Domain=" << domain;
			if (!expires.empty())   ss << "; Expires=" << expires;
			if (has_max_age)        ss << "; Max-Age=" << max_age;
			if (secure)             ss << "; Secure";
			if (http_only)          ss << "; HttpOnly";
			if (!same_site.empty()) ss << "; SameSite=" << same_site;
			return ss.str();
		}
	}
}
