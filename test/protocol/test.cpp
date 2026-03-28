#include "http_request.hpp"
#include <iostream>
#include <cassert>

using namespace http::core;

int main() {
    // Пример простого GET-запроса
    std::string get_request =
        "GET /index.html HTTP/1.1\n"
        "Host: example.com\n"
        "User-Agent: test-agent\n"
        "\r\n";

    std::pair<uint16_t, Request::__http_request> result = Request::parse_message(get_request);
    assert(result.first == 200);
    assert(result.second.method == types::GET);
    assert(result.second.uri == "/index.html");
    assert(result.second.version == "1.1");
    assert(result.second.headers["Host"][0] == "example.com");
    assert(result.second.body.empty());

    // Пример POST-запроса с телом
    std::string post_request =
        "POST /api HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Type: text/plain\n"
        "\r\n"
        "hello";

    std::pair<uint16_t, Request::__http_request> result2 = Request::parse_message(post_request);
    assert(result2.first == 200);
    assert(result2.second.method == types::POST);
    assert(result2.second.uri == "/api");
    assert(result2.second.version == "1.1");
    assert(result2.second.headers["Host"][0] == "example.com");
    assert(result2.second.body == "hello");

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
