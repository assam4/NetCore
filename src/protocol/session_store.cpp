#include "session_store.hpp"
# include <sstream>
# include <cstdlib>

namespace http {
	namespace core {

		unsigned long Sessions::_sid_seq = 0;

		Session::Session() : expires_at(0) {}

		Session::Session(const std::string& sid, time_t exp)
			: session_id(sid), expires_at(exp) {}

		bool Session::is_expired() const {
			return std::time(NULL) >= expires_at;
		}

		void Session::refresh(long max_age_seconds) {
			expires_at = std::time(NULL) + max_age_seconds;
		}

		Sessions::Sessions() : _max_age(SESSION_MAX_AGE) {}

		Sessions::Sessions(long max_age_seconds) : _max_age(max_age_seconds) {}

		std::deque<Session>::iterator Sessions::find_session(const std::string& sid) {
			for (std::deque<Session>::iterator it = _sessions.begin(); it != _sessions.end(); ++it) {
				if (it->session_id == sid)
					return it;
			}
			return _sessions.end();
		}

		std::string Sessions::generate_session_id() {
			std::ostringstream ss;
			ss << std::time(NULL) << "-" << std::rand() << "-" << ++_sid_seq;
			return ss.str();
		}

		bool Sessions::check_session(const std::map<std::string, std::string>& cookies) {
			std::map<std::string, std::string>::const_iterator sid_it = cookies.find("session_id");
			if (sid_it == cookies.end())
				return false;
			std::deque<Session>::iterator it = find_session(sid_it->second);
			if (it == _sessions.end())
				return false;
			if (it->is_expired()) {
				_sessions.erase(it);
				return false;
			}
			it->refresh(_max_age);
			return true;
		}

		std::string Sessions::ensure_session(const std::map<std::string, std::string>& cookies, bool& created) {
			created = false;
			std::map<std::string, std::string>::const_iterator sid_it = cookies.find("session_id");
			if (sid_it != cookies.end()) {
				std::deque<Session>::iterator it = find_session(sid_it->second);
				if (it != _sessions.end()) {
					if (!it->is_expired()) {
						it->refresh(_max_age);
						return it->session_id;
					}
					_sessions.erase(it);
				}
			}
			std::string sid = generate_session_id();
			Session s(sid, std::time(NULL) + _max_age);
			_sessions.push_back(s);
			created = true;
			return sid;
		}
	}
}
