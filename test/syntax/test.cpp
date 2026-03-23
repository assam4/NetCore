#include <iostream>
#include <set>
#include <map>
#include "tokenize_factory.hpp"
#include "configparser.hpp"

using namespace http::config::lexer;
using namespace http::config::parser;

void print_server(const __server_row_data& s) {
    if (!s.listen.empty()) {
        std::cout << "  Listen: ";
        for (std::set<std::string>::const_iterator it = s.listen.begin(); it != s.listen.end(); ++it)
            std::cout << *it << " ";
        std::cout << std::endl;
    }
    if (!s.server_name.empty()) {
        std::cout << "  Server names: ";
        for (std::set<std::string>::const_iterator it = s.server_name.begin(); it != s.server_name.end(); ++it)
            std::cout << *it << " ";
        std::cout << std::endl;
    }
    if (!s.root.empty())
        std::cout << "  Root: " << s.root << std::endl;
    if (!s.index.empty()) {
        std::cout << "  Index: ";
        for (std::set<std::string>::const_iterator it = s.index.begin(); it != s.index.end(); ++it)
            std::cout << *it << " ";
        std::cout << std::endl;
    }
    if (!s.allowed_methods.empty()) {
        std::cout << "  Allowed methods: ";
        for (std::set<std::string>::const_iterator it = s.allowed_methods.begin(); it != s.allowed_methods.end(); ++it)
            std::cout << *it << " ";
        std::cout << std::endl;
    }
    if (!s.error_pages.empty()) {
        std::cout << "  Error pages: ";
        for (std::map<std::set<std::string>, std::string>::const_iterator it = s.error_pages.begin(); it != s.error_pages.end(); ++it) {
            std::cout << "{";
            for (std::set<std::string>::const_iterator code = it->first.begin(); code != it->first.end(); ++code)
                std::cout << *code << " ";
            std::cout << "}:" << it->second << " ";
        }
        std::cout << std::endl;
    }
    if (!s.client_max_body_size.empty())
        std::cout << "  Client max body size: " << s.client_max_body_size << std::endl;
    if (!s.ret_redirection.first.empty())
        std::cout << "  Return: " << s.ret_redirection.first << " " << s.ret_redirection.second << std::endl;
    if (!s.autoindex.empty())
        std::cout << "  Autoindex: " << s.autoindex << std::endl;

    if (!s.locations.empty()) {
        std::cout << "  Locations: " << s.locations.size() << std::endl;
        for (std::size_t i = 0; i < s.locations.size(); ++i) {
            const __location_row_data& l = s.locations[i];
            std::cout << "    Location #" << i << ": " << l.modifier << " " << l.path << std::endl;
            if (!l.root.empty())
                std::cout << "      Root: " << l.root << std::endl;
            if (!l.index.empty()) {
                std::cout << "      Index: ";
                for (std::set<std::string>::const_iterator it = l.index.begin(); it != l.index.end(); ++it)
                    std::cout << *it << " ";
                std::cout << std::endl;
            }
            if (!l.allowed_methods.empty()) {
                std::cout << "      Allowed methods: ";
                for (std::set<std::string>::const_iterator it = l.allowed_methods.begin(); it != l.allowed_methods.end(); ++it)
                    std::cout << *it << " ";
                std::cout << std::endl;
            }
            if (!l.cgi_extension.empty()) {
                std::cout << "      CGI extension: ";
                for (std::set<std::string>::const_iterator it = l.cgi_extension.begin(); it != l.cgi_extension.end(); ++it)
                    std::cout << *it << " ";
                std::cout << std::endl;
            }
            if (!l.upload_location.empty())
                std::cout << "      Upload location: " << l.upload_location << std::endl;
        }
    }
}

bool check_test(const std::string& config, const std::string& label, bool expected_success) {
    std::cout << label << ": ";
    TokenFactory factory;
    factory.extractTokens(const_cast<std::string&>(config));
    std::vector<IToken*> tokens = factory.getTokens();

    bool actual_success = false;
    std::string error_msg;
    std::vector<__server_row_data> servers;

    try {
        servers = ConfigParser::parse(tokens);
        actual_success = true;
    } catch (const std::exception& e) {
        error_msg = e.what();
        actual_success = false;
    }

    bool test_passed = (actual_success == expected_success);
    if (test_passed) {
        std::cout << "\033[32msuccess\033[0m" << std::endl;
        if (actual_success) {
            for (std::size_t i = 0; i < servers.size(); ++i) {
                std::cout << "  [Server #" << i << "]" << std::endl;
                print_server(servers[i]);
            }
        }
    } else {
        std::cout << "\033[31mfailure\033[0m";
        if (!actual_success) {
            std::cout << " (" << error_msg << ")";
        }
        std::cout << std::endl;
    }

    return test_passed;
}

int main() {
    bool all_success = true;

    // Тесты на скобки
    if (!check_test("http { server { } }", "Valid: Basic http server block", true)) all_success = false;
    if (!check_test("http { server { location / { } } }", "Valid: Http with server and location", true)) all_success = false;
    if (!check_test("http { server { } server { } }", "Valid: Multiple servers", true)) all_success = false;
    if (!check_test("http { server { ", "Invalid: Unclosed server brace", false)) all_success = false;
    if (!check_test("http { server } }", "Invalid: Extra closing brace", false)) all_success = false;
    if (!check_test("http server { }", "Invalid: Missing opening brace after http", false)) all_success = false;
    if (!check_test("http { } server { }", "Invalid: Server outside http block", false)) all_success = false;
    if (!check_test("{ server { } }", "Invalid: Missing http keyword", false)) all_success = false;
    if (!check_test("http { server { } } }", "Invalid: Extra closing brace at end", false)) all_success = false;
    if (!check_test("http { server { location / {  }", "Invalid: Unclosed location brace", false)) all_success = false;
    if (!check_test("http { server { } } http { server { } }", "Invalid: Multiple http blocks", false)) all_success = false;
    if (!check_test("http { server { location /upload { upload_location /tmp/uploads; } } }", "Valid: Upload location directive", true)) all_success = false;
    // Тесты с конфигами
    if (!check_test("http { server { listen 8080; server_name example.com; root /var/www; index index.html; } }", "Valid: Full server config", true)) all_success = false;
    if (!check_test("http { server { listen 8080; location / { root /var/www; index index.html; } } }", "Valid: Server with location", true)) all_success = false;
    if (!check_test("http { server { listen 8080; error_page 404 /404.html; autoindex on; } }", "Valid: Server with error page and autoindex", true)) all_success = false;
    if (!check_test("http { server { listen 8080; error_page 500 502 503 504 /50x.html; } }", "Valid: Server with multi-code error_page", true)) all_success = false;

    if (!check_test("http { listen 8080; }", "Invalid: Listen outside server", false)) all_success = false;
    if (!check_test("http { server { root /var/www; location / { listen 8080; } } }", "Invalid: Listen in location", false)) all_success = false;
    if (!check_test("http { server { } location / { } }", "Invalid: Location outside server", false)) all_success = false;
    if (!check_test("http { server { location / { server { } } } }", "Invalid: Server inside location", false)) all_success = false;

    return all_success ? 0 : 1;
}
