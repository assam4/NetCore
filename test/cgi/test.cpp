#include "CGI.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include <iostream>

int main()
{
    // Create HTTP request object
    http::core::Request::__http_request req;
    req.method = http::core::types::GET;
    req.uri = "name=andranik";
    req.version = "HTTP/1.1";
    req.body = "";
    req.headers["Host"].push_back("localhost");
    req.headers["Content-Type"].push_back("text/plain");

    // Execute CGI script
    http::core::Response::_http_response resp = http::core::CGI::exec(
        req,
        "./test.py",
        "/usr/bin/python3"
    );

    // Serialize response to HTTP format
    std::string httpResponse = http::core::Response::serialize(resp);

    std::cout << "===== CGI RESPONSE =====" << std::endl;
    std::cout << httpResponse << std::endl;

    return 0;
}