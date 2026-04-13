#ifndef HTTP_COOKIE_HPP
# define HTTP_COOKIE_HPP

# include <map>
# include <string>
# include <ctime>

namespace http {
	namespace core {

    /**
     * @brief HTTP cookie model used for request lookup and response emission.
     * Holds the cookie name/value pair plus Set-Cookie attributes.
     * Provides helpers to populate from request cookies and serialize back.
     */
        class Cookie {
        public:
            std::string name;
            std::string value;
            std::string path;
            std::string domain;
            std::string expires;
            long max_age;
            bool has_max_age;
            bool secure;
            bool http_only;
            std::string same_site;

            Cookie(): max_age(0), has_max_age(false), secure(false), http_only(false) {}

            bool fill_from_request_map(const std::map<std::string, std::string>&, const std::string&);
            void set_session(const std::string&, long);
            void clear_session() { set_session("", 0); }

            std::string serialize_set_cookie() const;
        };

	}
}

#endif
