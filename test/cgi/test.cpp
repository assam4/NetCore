#include "CGI.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include <iostream>

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void print_request(const http::core::Request::__http_request& req, uint16_t port) {
    print_section("HTTP REQUEST");
    std::cout << "Method: " << (req.method == http::core::types::GET ? "GET" :
                               req.method == http::core::types::POST ? "POST" :
                               req.method == http::core::types::DEL ? "DELETE" : "UNKNOWN") << std::endl;
    std::cout << "URI: " << req.uri << std::endl;
    std::cout << "Version: " << req.version << std::endl;
    std::cout << "Port: " << port << std::endl;

    if (!req.body.empty())
        std::cout << "Body: " << req.body << std::endl;

    std::cout << "\nHeaders:" << std::endl;
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = req.headers.begin();
         it != req.headers.end(); ++it) {
        if (!it->second.empty())
            std::cout << "  " << it->first << ": " << it->second[0] << std::endl;
    }
}

void print_response(const http::core::Response::_http_response& resp) {
    print_section("CGI RESPONSE");
    std::cout << "Status: " << resp._status << " " << http::core::types::StatusRegistry::get_phrase(resp._status) << std::endl;
    std::cout << "Version: " << resp._version << std::endl;

    std::cout << "\nResponse Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = resp._headers.begin();
         it != resp._headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }

    if (!resp._body.empty()) {
        std::cout << "\nResponse Body (" << resp._body.length() << " bytes):" << std::endl;
        std::cout << resp._body << std::endl;
    }
}

void test_example_1() {
    std::cout << "\n\n";
    print_section("EXAMPLE 1: GET request with query parameters");

    http::core::Request::__http_request req;
    req.method = http::core::types::GET;
    req.uri = "/search.cgi?q=webserv&limit=10";
    req.version = "HTTP/1.1";
    req.body = "";
    req.headers["Host"].push_back("example.com:8080");
    req.headers["User-Agent"].push_back("Mozilla/5.0");
    req.headers["Accept"].push_back("text/html");

    print_request(req, 8080);

    http::core::Response::_http_response resp = http::core::CGI::exec(req, "./test.py", "/usr/bin/python3", 8080);
    print_response(resp);
}

void test_example_2() {
    std::cout << "\n\n";
    print_section("EXAMPLE 2: POST request with form data");

    http::core::Request::__http_request req;
    req.method = http::core::types::POST;
    req.uri = "/login.cgi";
    req.version = "HTTP/1.1";
    req.body = "username=admin&password=secret";
    req.headers["Host"].push_back("api.example.com:9000");
    req.headers["Content-Type"].push_back("application/x-www-form-urlencoded");
    req.headers["User-Agent"].push_back("curl/7.68.0");

    print_request(req, 9000);

    http::core::Response::_http_response resp = http::core::CGI::exec(req, "./test.py", "/usr/bin/python3", 9000);
    print_response(resp);
}

void test_example_3() {
    std::cout << "\n\n";
    print_section("EXAMPLE 3: DELETE request");

    http::core::Request::__http_request req;
    req.method = http::core::types::DEL;
    req.uri = "/api/resource/12345";
    req.version = "HTTP/1.1";
    req.body = "";
    req.headers["Host"].push_back("api.server.com:3000");
    req.headers["Authorization"].push_back("Bearer token123");

    print_request(req, 3000);

    http::core::Response::_http_response resp = http::core::CGI::exec(req, "./test.py", "/usr/bin/python3", 3000);
    print_response(resp);
}

void test_example_4() {
    std::cout << "\n\n";
    print_section("EXAMPLE 4: Multiple request headers");

    http::core::Request::__http_request req;
    req.method = http::core::types::GET;
    req.uri = "/page.cgi?lang=en";
    req.version = "HTTP/1.1";
    req.body = "";
    req.headers["Host"].push_back("localhost:8080");
    req.headers["User-Agent"].push_back("Mozilla/5.0 (Linux; X11)");
    req.headers["Accept"].push_back("text/html");
    req.headers["Accept-Language"].push_back("en-US,en;q=0.9");
    req.headers["Referer"].push_back("http://localhost:8080/index.html");

    print_request(req, 8080);

    http::core::Response::_http_response resp = http::core::CGI::exec(req, "./test.py", "/usr/bin/python3", 8080);
    print_response(resp);
}

int main() {
    std::cout << "\n";
    std::cout << " ███████████████████████████████████████████████████████████████" << std::endl;
    std::cout << " █  CGI Execution Examples - webserv Project                     █" << std::endl;
    std::cout << " ███████████████████████████████████████████████████████████████" << std::endl;

    try {
        test_example_1();
        test_example_2();
        test_example_3();
        test_example_4();

        // Test error handling
        std::cout << "\n\n";
        print_section("ERROR HANDLING: Script file not found (404)");
        http::core::Request::__http_request error_req;
        error_req.method = http::core::types::GET;
        error_req.uri = "/missing.cgi";
        error_req.version = "HTTP/1.1";
        error_req.body = "";
        error_req.headers["Host"].push_back("localhost:8080");

        print_request(error_req, 8080);

        // Test with non-existent script file
        http::core::Response::_http_response error_resp = http::core::CGI::exec(error_req, "./notexist.py", "/usr/bin/python3", 8080);
        print_response(error_resp);

        // Test error handling 2
        std::cout << "\n\n";
        print_section("ERROR HANDLING: Interpreter not found (404)");
        http::core::Request::__http_request error_req2;
        error_req2.method = http::core::types::GET;
        error_req2.uri = "/test.cgi";
        error_req2.version = "HTTP/1.1";
        error_req2.body = "";
        error_req2.headers["Host"].push_back("localhost:8080");

        print_request(error_req2, 8080);

        // Use non-existent interpreter to trigger execve error
        http::core::Response::_http_response error_resp2 = http::core::CGI::exec(error_req2, "./test.py", "/nonexistent/python", 8080);
        print_response(error_resp2);

        std::cout << "\n";
        std::cout << std::string(70, '=') << std::endl;
        std::cout << std::string(70, '=') << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
