#ifndef HTTP_TRANSACTION_HPP
#define HTTP_TRANSACTION_HPP

#include <map>
#include <string>
#include "virtualhost.hpp"
#include "http_request.hpp"
#include "http_types.hpp"

namespace http {
    namespace core {

		class HttpTransaction {
			private:
				HttpTransaction();
				HttpTransaction(const HttpTransaction&);
				HttpTransaction& operator=(const HttpTransaction&);
			public:
				static const types::__location& get_best_location(const VirtualHost& vhost, const std::string& uri_path);
				static types::HttpStatus validate_location_rules(const Request& req, const types::__location& loc);
		};

    }
}

#endif
