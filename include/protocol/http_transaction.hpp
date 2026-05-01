#ifndef HTTP_TRANSACTION_HPP
#define HTTP_TRANSACTION_HPP

#include <map>
#include <string>
#include "virtualhost.hpp"
#include "http_request.hpp"
#include "http_types.hpp"
#include "HttpServer.hpp"

namespace http {
    namespace core {

		class HttpTransaction {
			private:
				HttpTransaction();
				HttpTransaction(const HttpTransaction&);
				HttpTransaction& operator=(const HttpTransaction&);
			public:
				static void prepare(Connection* _conn, HttpServer& _http_server,  std::pair<types::HttpStatus, Request>& parsed_req);
				static const types::__location& get_best_location(const VirtualHost& vhost, const std::string& uri_path);
				static bool process(Connection *_conn, HttpServer& http_server);
		};
	}
}

#endif
