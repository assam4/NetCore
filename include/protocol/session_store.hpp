#ifndef SESSION_STORE_HPP
# define SESSION_STORE_HPP

# include <map>
# include <string>
# include <deque>
# include <ctime>

namespace http {
	namespace core {

		const long SESSION_MAX_AGE = 3600;

		/**
		 * @brief One active server-side session record.
		 * Stores the opaque session identifier and its absolute expiry time.
		 * Used internally by the session registry to validate incoming cookies.
		 */
		struct Session {
			std::string session_id;
			time_t expires_at;

			Session();
			Session(const std::string&, time_t);
			bool is_expired() const;
			void refresh(long max_age_seconds);
		};

		/**
		 * @brief In-memory session registry for cookie-backed authentication.
		 * Searches request cookies for a session_id and validates expiry.
		 * Can also mint a new session identifier and register its lifetime.
		 */
		class Sessions {
		private:
			std::deque<Session> _sessions;
			long _max_age;
			static unsigned long _sid_seq;

			std::deque<Session>::iterator find_session(const std::string&);
			std::string generate_session_id();
		public:
			Sessions();
			explicit Sessions(long);
			bool check_session(const std::map<std::string, std::string>&);
			std::string ensure_session(const std::map<std::string, std::string>&, bool&);
		};

	}
}

#endif
