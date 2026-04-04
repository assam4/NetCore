#include "http_request.hpp"
#include <iostream>
#include <string>

using namespace http::core;

static bool expect_eq(const std::string& label, const std::string& actual, const std::string& expected) {
    if (actual != expected) {
        std::cout << "[FAIL] " << label << " expected='" << expected << "' actual='" << actual << "'\n";
        return false;
    }
    std::cout << "[OK]   " << label << " = '" << actual << "'\n";
    return true;
}

static bool expect_eq(const std::string& label, uint16_t actual, uint16_t expected) {
    if (actual != expected) {
        std::cout << "[FAIL] " << label << " expected=" << expected << " actual=" << actual << "\n";
        return false;
    }
    std::cout << "[OK]   " << label << " = " << actual << "\n";
    return true;
}

static bool expect_eq(const std::string& label, uint8_t actual, uint8_t expected) {
    if (actual != expected) {
        std::cout << "[FAIL] " << label << " expected=" << static_cast<int>(expected)
                  << " actual=" << static_cast<int>(actual) << "\n";
        return false;
    }
    std::cout << "[OK]   " << label << " = " << static_cast<int>(actual) << "\n";
    return true;
}

static std::string get_first_header(const Request::__http_request& hr, const std::string& key) {
    std::map<std::string, std::vector<std::string> >::const_iterator it = hr.headers.find(key);
    if (it == hr.headers.end() || it->second.empty())
        return "";
    return it->second[0];
}

static std::string get_query_value(const Request::__http_request& hr, const std::string& key) {
    std::map<std::string, std::string>::const_iterator it = hr.query.find(key);
    if (it == hr.query.end())
        return "";
    return it->second;
}

static void print_parsed(const std::string& title, const Request::__http_request& hr) {
    std::cout << "\n--- " << title << " parsed data ---\n";
    std::cout << "method: " << static_cast<int>(hr.method) << "\n";
    std::cout << "version: " << hr.version << "\n";
    std::cout << "uri: " << hr.uri << "\n";
    std::cout << "body: '" << hr.body << "'\n";
    if (hr.headers.count("Host") && !hr.headers.find("Host")->second.empty()) {
        std::cout << "Host: " << hr.headers.find("Host")->second[0] << "\n";
    }
    std::cout << "query params:\n";
    if (hr.query.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (std::map<std::string, std::string>::const_iterator it = hr.query.begin(); it != hr.query.end(); ++it) {
            std::cout << "  " << it->first << " = " << it->second << "\n";
        }
    }
}

int main() {
    bool ok = true;

    const std::string origin_request =
        "GET /index.html?lang=ru&page=2 HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";

    const std::pair<uint16_t, Request::__http_request> origin_result = Request::parse_message(origin_request, 1024);
    ok = expect_eq("origin status", origin_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("origin method", origin_result.second.method, static_cast<uint8_t>(types::GET)) && ok;
    ok = expect_eq("origin uri", origin_result.second.uri, "/index.html") && ok;
    ok = expect_eq("origin version", origin_result.second.version, "1.1") && ok;
    ok = expect_eq("origin host", get_first_header(origin_result.second, "Host"), "example.com") && ok;
    ok = expect_eq("origin query lang", get_query_value(origin_result.second, "lang"), "ru") && ok;
    ok = expect_eq("origin query page", get_query_value(origin_result.second, "page"), "2") && ok;

    const std::string absolute_request =
        "GET http://example.com/products/list?cat=books&sort=asc HTTP/1.0\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";

    const std::pair<uint16_t, Request::__http_request> absolute_result = Request::parse_message(absolute_request, 1024);
    ok = expect_eq("absolute status", absolute_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("absolute method", absolute_result.second.method, static_cast<uint8_t>(types::GET)) && ok;
    ok = expect_eq("absolute uri", absolute_result.second.uri,
                   "http://example.com/products/list") && ok;
    ok = expect_eq("absolute version", absolute_result.second.version, "1.0") && ok;
    ok = expect_eq("absolute query cat", get_query_value(absolute_result.second, "cat"), "books") && ok;
    ok = expect_eq("absolute query sort", get_query_value(absolute_result.second, "sort"), "asc") && ok;

    // Tricky negative cases: try to break parser and verify status codes.
    const std::string no_host_http11 =
        "GET /index.html HTTP/1.1\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";
    ok = expect_eq("bad no host (http/1.1)", Request::parse_message(no_host_http11, 1024).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string broken_query =
        "GET /search?onlykey HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("bad query without '='", Request::parse_message(broken_query, 1024).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string bad_absolute_no_path =
        "GET http://example.com HTTP/1.0\r\n"
        "\r\n";
    ok = expect_eq("bad absolute without path", Request::parse_message(bad_absolute_no_path, 1024).first,
                   static_cast<uint16_t>(400)) && ok;

    std::string long_uri(4097, 'a');
    const std::string too_long_uri =
        std::string("GET /") + long_uri + " HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("too long uri", Request::parse_message(too_long_uri, 1024).first,
                   static_cast<uint16_t>(414)) && ok;

    std::string huge_header_value(8200, 'b');
    const std::string huge_header =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "X-Long: " + huge_header_value + "\r\n"
        "\r\n";
    ok = expect_eq("too long header", Request::parse_message(huge_header, 1024).first,
                   static_cast<uint16_t>(431)) && ok;

    const std::string too_big_body =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n"
        "0123456789ABCDEF";
    ok = expect_eq("too big body", Request::parse_message(too_big_body, 8).first,
                   static_cast<uint16_t>(413)) && ok;

    const std::string bad_version =
        "GET / HTTP/1.9\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("bad http version", Request::parse_message(bad_version, 1024).first,
                   static_cast<uint16_t>(505)) && ok;

    print_parsed("origin-form", origin_result.second);
    print_parsed("absolute-form", absolute_result.second);

    std::cout << "\nResult: " << (ok ? "ALL CHECKS PASSED" : "HAS FAILURES") << "\n";
    return ok ? 0 : 1;
}
