#ifndef HTTP_RESPONSE
# define HTTP_RESPONSE

#include <map>
#include "http_types.hpp"
#include <string>
#include <stdint.h>

namespace http {
	namespace core {

		class StatusRegistry {
			private:
				std::map<types::HttpStatus, std::string> _phrases;

				StatusRegistry();

				StatusRegistry(const StatusRegistry&);
				StatusRegistry& operator=(const StatusRegistry&);
			public:
				static StatusRegistry& instance();
				std::string get_phrase(types::HttpStatus status) const;
		};
	}
}

#endif
