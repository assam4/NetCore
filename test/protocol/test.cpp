#include "http_request.hpp"
#include "http_response.hpp"
#include "Server.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

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

static bool expect_eq_size(const std::string& label, size_t actual, size_t expected) {
    if (actual != expected) {
        std::cout << "[FAIL] " << label << " expected=" << expected << " actual=" << actual << "\n";
        return false;
    }
    std::cout << "[OK]   " << label << " = " << actual << "\n";
    return true;
}

static bool expect_true(const std::string& label, bool condition) {
    if (!condition) {
        std::cout << "[FAIL] " << label << " expected=true actual=false\n";
        return false;
    }
    std::cout << "[OK]   " << label << " = true\n";
    return true;
}

static bool expect_contains(const std::string& label, const std::string& text, const std::string& needle) {
    if (text.find(needle) == std::string::npos) {
        std::cout << "[FAIL] " << label << " missing='" << needle << "'\n";
        return false;
    }
    std::cout << "[OK]   " << label << " contains='" << needle << "'\n";
    return true;
}

static bool expect_cookie(const std::string& label, const Request& req,
                          const std::string& key, const std::string& expected) {
    std::map<std::string, std::string>::const_iterator it = req.headers.cookies.find(key);
    if (it == req.headers.cookies.end()) {
        std::cout << "[FAIL] " << label << " missing cookie='" << key << "'\n";
        return false;
    }
    if (it->second != expected) {
        std::cout << "[FAIL] " << label << " cookie='" << key
                  << "' expected='" << expected << "' actual='" << it->second << "'\n";
        return false;
    }
    std::cout << "[OK]   " << label << " cookie='" << key << "' value='" << it->second << "'\n";
    return true;
}

static bool send_all(int fd, const std::string& data) {
    size_t total = 0;
    while (total < data.size()) {
        ssize_t n = ::send(fd, data.c_str() + total, data.size() - total, 0);
        if (n <= 0)
            return false;
        total += static_cast<size_t>(n);
    }
    return true;
}

static std::pair<types::HttpStatus, Request> parse_with_connection(const std::string& head, const std::string& body = "") {
    int fds[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
        throw std::runtime_error("socketpair failed");

    if (!send_all(fds[1], head)) {
        ::close(fds[0]);
        ::close(fds[1]);
        throw std::runtime_error("send head failed");
    }

    Connection conn(fds[0], 8080);

    if (conn.read_once() <= 0) {
        ::close(fds[1]);
        throw std::runtime_error("read_once failed");
    }

    pid_t sender_pid = -1;
    if (!body.empty()) {
        sender_pid = ::fork();
        if (sender_pid < 0) {
            ::close(fds[1]);
            throw std::runtime_error("fork failed");
        }
        if (sender_pid == 0) {
            bool ok = send_all(fds[1], body);
            ::shutdown(fds[1], SHUT_WR);
            ::close(fds[1]);
            ::_exit(ok ? 0 : 1);
        }
    } else {
        ::shutdown(fds[1], SHUT_WR);
    }

    std::pair<types::HttpStatus, Request> out = Request::parse_message(conn);
    ::close(fds[1]);

    if (sender_pid > 0) {
        int status = 0;
        ::waitpid(sender_pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            throw std::runtime_error("send body failed");
    }
    return out;
}

static std::pair<types::HttpStatus, Request> parse_with_parts(
    const std::string& head,
    const std::vector<std::string>& parts,
    useconds_t delay_us)
{
    int fds[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
        throw std::runtime_error("socketpair failed");

    if (!send_all(fds[1], head)) {
        ::close(fds[0]);
        ::close(fds[1]);
        throw std::runtime_error("send head failed");
    }

    Connection conn(fds[0], 8080);
    if (conn.read_once() <= 0) {
        ::close(fds[1]);
        throw std::runtime_error("read_once failed");
    }

    pid_t sender_pid = -1;
    if (!parts.empty()) {
        sender_pid = ::fork();
        if (sender_pid < 0) {
            ::close(fds[1]);
            throw std::runtime_error("fork failed");
        }
        if (sender_pid == 0) {
            bool ok = true;
            for (size_t i = 0; i < parts.size() && ok; ++i) {
                ok = send_all(fds[1], parts[i]);
                if (ok && delay_us && (i + 1 < parts.size()))
                    usleep(delay_us);
            }
            ::shutdown(fds[1], SHUT_WR);
            ::close(fds[1]);
            ::_exit(ok ? 0 : 1);
        }
    } else {
        ::shutdown(fds[1], SHUT_WR);
    }

    std::pair<types::HttpStatus, Request> out = Request::parse_message(conn);
    ::close(fds[1]);

    if (sender_pid > 0) {
        int status = 0;
        ::waitpid(sender_pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            throw std::runtime_error("send parts failed");
    }
    return out;
}

static bool parse_with_parts_finishes(
    const std::string& head,
    const std::vector<std::string>& parts,
    useconds_t delay_us,
    unsigned timeout_ms,
    uint16_t& status_out)
{
    int pipefd[2];
    if (::pipe(pipefd) != 0)
        throw std::runtime_error("pipe failed");

    pid_t pid = ::fork();
    if (pid < 0) {
        ::close(pipefd[0]);
        ::close(pipefd[1]);
        throw std::runtime_error("fork failed");
    }

    if (pid == 0) {
        ::close(pipefd[0]);
        uint16_t code = static_cast<uint16_t>(500);
        try {
            std::pair<types::HttpStatus, Request> out = parse_with_parts(head, parts, delay_us);
            code = static_cast<uint16_t>(out.first);
        } catch (...) {
            code = static_cast<uint16_t>(500);
        }
        (void)::write(pipefd[1], &code, sizeof(code));
        ::close(pipefd[1]);
        ::_exit(0);
    }

    ::close(pipefd[1]);
    unsigned elapsed = 0;
    int status = 0;
    while (elapsed < timeout_ms) {
        pid_t w = ::waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            uint16_t code = 500;
            ssize_t n = ::read(pipefd[0], &code, sizeof(code));
            ::close(pipefd[0]);
            if (n == static_cast<ssize_t>(sizeof(code)))
                status_out = code;
            else
                status_out = 500;
            return true;
        }
        usleep(1000);
        ++elapsed;
    }

    ::kill(pid, SIGKILL);
    ::waitpid(pid, &status, 0);
    ::close(pipefd[0]);
    return false;
}

static std::string get_query_value(const Request& req, const std::string& key) {
    if (req.start_line.query.empty())
        return "";

    std::string search = key + "=";
    size_t pos = req.start_line.query.find(search);
    if (pos == std::string::npos)
        return "";

    pos += search.length();
    size_t end = req.start_line.query.find('&', pos);
    if (end == std::string::npos)
        end = req.start_line.query.length();
    return req.start_line.query.substr(pos, end - pos);
}

int main() {
    bool ok = true;

    const std::string origin_head =
        "GET /index.html?lang=ru&page=2 HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";

    std::pair<types::HttpStatus, Request> origin_result = parse_with_connection(origin_head);
    ok = expect_eq("origin status", origin_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("origin method", origin_result.second.start_line.method, static_cast<uint8_t>(types::GET)) && ok;
    ok = expect_eq("origin uri", origin_result.second.start_line.uri, "/index.html") && ok;
    ok = expect_eq("origin version", origin_result.second.start_line.version, "HTTP/1.1") && ok;
    ok = expect_eq("origin query lang", get_query_value(origin_result.second, "lang"), "ru") && ok;
    ok = expect_eq("origin query page", get_query_value(origin_result.second, "page"), "2") && ok;

    const std::string absolute_head =
        "GET http://example.com/products/list?cat=books&sort=asc HTTP/1.0\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";

    std::pair<types::HttpStatus, Request> absolute_result = parse_with_connection(absolute_head);
    ok = expect_eq("absolute status", absolute_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("absolute method", absolute_result.second.start_line.method, static_cast<uint8_t>(types::GET)) && ok;
    ok = expect_eq("absolute uri", absolute_result.second.start_line.uri, "/products/list") && ok;
    ok = expect_eq("absolute version", absolute_result.second.start_line.version, "HTTP/1.0") && ok;
    ok = expect_eq("absolute query cat", get_query_value(absolute_result.second, "cat"), "books") && ok;
    ok = expect_eq("absolute query sort", get_query_value(absolute_result.second, "sort"), "asc") && ok;

    const std::string no_host_http11 =
        "GET /index.html HTTP/1.1\r\n"
        "User-Agent: test-agent\r\n"
        "\r\n";
    ok = expect_eq("bad no host (http/1.1)", parse_with_connection(no_host_http11).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string good_fixed_head =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 3\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> fixed_result = parse_with_connection(good_fixed_head, "abc");
    ok = expect_eq("fixed body status", fixed_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("fixed body content", fixed_result.second.body.content, "abc") && ok;

    const std::string short_fixed_head =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 10\r\n"
        "\r\n";
    ok = expect_eq("body shorter than content-length", parse_with_connection(short_fixed_head, "abc").first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string bad_chunked_value_head =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: notchunked\r\n"
        "\r\n";
    ok = expect_eq("bad transfer-encoding value", parse_with_connection(bad_chunked_value_head, "abc").first,
                   static_cast<uint16_t>(501)) && ok;

    const std::string bad_chunk_size_head =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    ok = expect_eq("bad chunk size (not hex)", parse_with_connection(bad_chunk_size_head, "nothex\r\nabc\r\n0\r\n\r\n").first,
                   static_cast<uint16_t>(400)) && ok;

    // HTTP/1.1 + Content-Length: valid header, body must be fully read
    const std::string post_11_len_head =
        "POST /v11-len HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 5\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> post_11_len = parse_with_connection(post_11_len_head, "hello");
    ok = expect_eq("http/1.1 content-length status", post_11_len.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("http/1.1 content-length body", post_11_len.second.body.content, "hello") && ok;

    // HTTP/1.1 + Transfer-Encoding: chunked, body must be decoded correctly
    const std::string post_11_chunked_head =
        "POST /v11-chunked HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> post_11_chunked =
        parse_with_connection(post_11_chunked_head, "3\r\nabc\r\n2\r\nde\r\n0\r\n\r\n");
    ok = expect_eq("http/1.1 chunked status", post_11_chunked.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("http/1.1 chunked body", post_11_chunked.second.body.content, "abcde") && ok;

    // HTTP/1.0 + Content-Length: valid and should read fixed-size body
    const std::string post_10_len_head =
        "POST /v10-len HTTP/1.0\r\n"
        "Content-Length: 4\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> post_10_len = parse_with_connection(post_10_len_head, "test");
    ok = expect_eq("http/1.0 content-length status", post_10_len.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("http/1.0 content-length body", post_10_len.second.body.content, "test") && ok;

    // HTTP/1.0 + Transfer-Encoding: chunked: accepted by current parser and decoded
    const std::string post_10_chunked_head =
        "POST /v10-chunked HTTP/1.0\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> post_10_chunked =
        parse_with_connection(post_10_chunked_head, "2\r\nhi\r\n1\r\n!\r\n0\r\n\r\n");
    ok = expect_eq("http/1.0 chunked status", post_10_chunked.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("http/1.0 chunked body", post_10_chunked.second.body.content, "hi!") && ok;

    // HTTP/1.1 POST without both Content-Length and Transfer-Encoding -> LENGTH_REQUIRED
    const std::string post_11_no_len_no_te =
        "POST /v11-nobody-meta HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("http/1.1 post without length/te", parse_with_connection(post_11_no_len_no_te).first,
                   static_cast<uint16_t>(411)) && ok;

    // HTTP/1.0 POST without both Content-Length and Transfer-Encoding -> LENGTH_REQUIRED (current logic)
    const std::string post_10_no_len_no_te =
        "POST /v10-nobody-meta HTTP/1.0\r\n"
        "\r\n";
    ok = expect_eq("http/1.0 post without length/te", parse_with_connection(post_10_no_len_no_te).first,
                   static_cast<uint16_t>(411)) && ok;

    // Header validation around body metadata: duplicate Content-Length should be rejected
    const std::string dup_content_length =
        "POST /dup-cl HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 3\r\n"
        "Content-Length: 3\r\n"
        "\r\n";
    ok = expect_eq("duplicate content-length", parse_with_connection(dup_content_length, "abc").first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string good_chunked_head =
        "POST /chunk HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> chunked_result =
        parse_with_connection(good_chunked_head, "4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n");
    ok = expect_eq("good chunked status", chunked_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("good chunked body", chunked_result.second.body.content, "Wikipedia") && ok;

    const std::string good_post_empty_body_head =
        "POST /empty HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> empty_post_result = parse_with_connection(good_post_empty_body_head);
    ok = expect_eq("good empty post status", empty_post_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("good empty post body", empty_post_result.second.body.content, "") && ok;

    const std::string bad_duplicate_host =
        "GET /dup HTTP/1.1\r\n"
        "Host: one.example\r\n"
        "Host: two.example\r\n"
        "\r\n";
    ok = expect_eq("bad duplicate host", parse_with_connection(bad_duplicate_host).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string bad_invalid_query =
        "GET /search?x=%GG HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("bad invalid query percent-encoding", parse_with_connection(bad_invalid_query).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string cookie_valid =
        "GET /cookie-ok HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Cookie: sid=abc123; theme=dark; lang=ru\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> cookie_valid_result = parse_with_connection(cookie_valid);
    ok = expect_eq("cookie valid status", cookie_valid_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq_size("cookie valid count", cookie_valid_result.second.headers.cookies.size(), 3) && ok;
    ok = expect_cookie("cookie sid", cookie_valid_result.second, "sid", "abc123") && ok;
    ok = expect_cookie("cookie theme", cookie_valid_result.second, "theme", "dark") && ok;
    ok = expect_cookie("cookie lang", cookie_valid_result.second, "lang", "ru") && ok;

    const std::string cookie_spaces_and_duplicate =
        "GET /cookie-dup HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Cookie: sid=one ; theme = dark ; sid = two\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> cookie_dup_result = parse_with_connection(cookie_spaces_and_duplicate);
    ok = expect_eq("cookie duplicate status", cookie_dup_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_cookie("cookie duplicate sid last wins", cookie_dup_result.second, "sid", "two") && ok;
    ok = expect_cookie("cookie duplicate theme", cookie_dup_result.second, "theme", "dark") && ok;

    const std::string cookie_invalid_name =
        "GET /cookie-bad-name HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Cookie: bad@name=1\r\n"
        "\r\n";
    ok = expect_eq("cookie invalid name", parse_with_connection(cookie_invalid_name).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string cookie_missing_equal =
        "GET /cookie-missing-eq HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Cookie: sid=1; broken\r\n"
        "\r\n";
    ok = expect_eq("cookie missing equal", parse_with_connection(cookie_missing_equal).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string cookie_for_response_helpers =
        "GET /cookie-use HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Cookie: sid=abc123; theme=dark\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> cookie_for_response = parse_with_connection(cookie_for_response_helpers);
    ok = expect_eq("cookie for response status", cookie_for_response.first, static_cast<uint16_t>(200)) && ok;
    Cookie sid_cookie;
    bool sid_found = sid_cookie.fill_from_request_map(cookie_for_response.second.headers.cookies, "sid");
    ok = expect_true("cookie method fill sid found", sid_found) && ok;
    ok = expect_eq("cookie method fill sid name", sid_cookie.name, "sid") && ok;
    ok = expect_eq("cookie method fill sid value", sid_cookie.value, "abc123") && ok;

    Cookie request_cookie;
    bool request_cookie_found = request_cookie.fill_from_request_map(cookie_for_response.second.headers.cookies, "sid");
    ok = expect_true("fill cookie from request map found", request_cookie_found) && ok;
    ok = expect_eq("fill cookie from request map name", request_cookie.name, "sid") && ok;
    ok = expect_eq("fill cookie from request map value", request_cookie.value, "abc123") && ok;

    Cookie missing_cookie;
    bool missing_found = missing_cookie.fill_from_request_map(cookie_for_response.second.headers.cookies, "missing");
    ok = expect_true("cookie method missing cookie not found", !missing_found) && ok;

    Response::_http_response response_cookie_out;
    response_cookie_out._version = "HTTP/1.1";
    response_cookie_out._status = types::OK;
    response_cookie_out._headers["Content-Length"] = "0";

    Cookie custom;
    custom.name = "theme";
    custom.value = "dark";
    custom.path = "/";
    custom.http_only = true;
    response_cookie_out._cookies.push_back(custom);

    Cookie session_cookie;
    session_cookie.set_session("newsid", 3600);
    response_cookie_out._cookies.push_back(session_cookie);

    Cookie clear_cookie;
    clear_cookie.clear_session();
    response_cookie_out._cookies.push_back(clear_cookie);

    ok = expect_eq_size("response cookie count", response_cookie_out._cookies.size(), 3) && ok;

    std::string serialized_response = Response::serialize(response_cookie_out);
    ok = expect_contains("response has theme set-cookie", serialized_response, "Set-Cookie: theme=dark; Path=/; HttpOnly") && ok;
    ok = expect_contains("response has sid set-cookie", serialized_response, "Set-Cookie: session_id=newsid; Path=/; Max-Age=3600; HttpOnly; SameSite=Lax") && ok;
    ok = expect_contains("response has sid clear-cookie", serialized_response, "Set-Cookie: session_id=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax") && ok;

    // Long body tests with different Content-Length values
    std::string long_body_exact(12000, 'x');
    std::stringstream cl_exact_ss;
    cl_exact_ss << long_body_exact.size();
    const std::string long_exact_head =
        "POST /long-exact HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: " + cl_exact_ss.str() + "\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> long_exact_result = parse_with_connection(long_exact_head, long_body_exact);
    ok = expect_eq("long body exact content-length status", long_exact_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq_size("long body exact read size", long_exact_result.second.body.content.size(), long_body_exact.size()) && ok;

    std::string long_body_short(8000, 'y');
    std::stringstream cl_more_ss;
    cl_more_ss << (long_body_short.size() + 2000);
    const std::string long_shorter_than_cl_head =
        "POST /long-short HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: " + cl_more_ss.str() + "\r\n"
        "\r\n";
    ok = expect_eq("long body shorter than content-length status",
                   parse_with_connection(long_shorter_than_cl_head, long_body_short).first,
                   static_cast<uint16_t>(400)) && ok;

    std::string long_body_longer(10000, 'z');
    std::stringstream cl_less_ss;
    cl_less_ss << 6000;
    const std::string long_longer_than_cl_head =
        "POST /long-long HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: " + cl_less_ss.str() + "\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> long_longer_result = parse_with_connection(long_longer_than_cl_head, long_body_longer);
    ok = expect_eq("long body longer than content-length status", long_longer_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq_size("long body longer than content-length read size", long_longer_result.second.body.content.size(), 6000) && ok;

    // --- Probes for reported analyzer findings ---

    const std::string bug1_protocol_suffix =
        "GET / HTTP/1.1X\r\n"
        "Host: example.com\r\n"
        "\r\n";
    ok = expect_eq("probe bug1 protocol suffix", parse_with_connection(bug1_protocol_suffix).first,
                   static_cast<uint16_t>(505)) && ok;

    const std::string bug3_incomplete_headers =
        "GET /partial HTTP/1.1\r\n"
        "Host: example.com";
    ok = expect_eq("probe bug3 incomplete headers accepted", parse_with_connection(bug3_incomplete_headers).first,
                   static_cast<uint16_t>(400)) && ok;

    const std::string bug4_priority_te_vs_cl =
        "POST /priority HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: notchunked\r\n"
        "Content-Length: 4\r\n"
        "\r\n";
    ok = expect_eq("probe bug4 transfer-encoding priority", parse_with_connection(bug4_priority_te_vs_cl, "test").first,
                   static_cast<uint16_t>(501)) && ok;

    const std::string bug5_query_basic =
        "GET /search?q=hello HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    std::pair<types::HttpStatus, Request> bug5_query_result = parse_with_connection(bug5_query_basic);
    ok = expect_eq("probe bug5 status", bug5_query_result.first, static_cast<uint16_t>(200)) && ok;
    ok = expect_eq("probe bug5 query parsed", get_query_value(bug5_query_result.second, "q"), "hello") && ok;

    const std::string bug2_chunk_head =
        "POST /split HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::vector<std::string> bug2_parts;
    bug2_parts.push_back("A\r");
    bug2_parts.push_back("\n1234567890\r\n0\r\n\r\n");
    uint16_t bug2_status = 0;
    bool bug2_finished = parse_with_parts_finishes(bug2_chunk_head, bug2_parts, 0, 1000, bug2_status);
    ok = expect_true("probe bug2 split-chunk parser finishes", bug2_finished) && ok;
    if (bug2_finished)
        ok = expect_eq("probe bug2 split-chunk status", bug2_status, static_cast<uint16_t>(200)) && ok;

    std::cout << "\nResult: " << (ok ? "ALL CHECKS PASSED" : "HAS FAILURES") << "\n";
    return ok ? 0 : 1;
}
