#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include "tokenize_factory.hpp"
#include "configparser.hpp"
#include "virtualhost.hpp"

using namespace http::config::lexer;
using namespace http::config::parser;
using namespace http::core;

static std::vector<VirtualHost> build_hosts(const std::vector<__server_row_data>& raw) {
    std::vector<VirtualHost> result;
    for (std::vector<__server_row_data>::const_iterator it = raw.begin(); it != raw.end(); ++it)
        result.push_back(VirtualHost::build(*it, result));
    return result;
}

static bool expect_true(bool cond, const std::string& name) {
    std::cout << name << ": " << (cond ? "PASS" : "FAIL") << std::endl;
    return cond;
}

static bool test_single_virtualhost_defaults() {
    const std::string cfg =
        "http { server { listen 8080; server_name demo.local; } }";

    TokenFactory factory;
    std::string data = cfg;
    factory.extractTokens(data);
    std::vector<IToken*> tokens = factory.getTokens();
    std::vector<__server_row_data> raw = ConfigParser::parse(tokens);
    std::vector<VirtualHost> hosts = build_hosts(raw);

    bool ok = true;
    ok &= expect_true(hosts.size() == 1, "single host count");
    ok &= expect_true(hosts[0].get_port() == 8080, "listen port");
    ok &= expect_true(hosts[0].get_root() == "html", "default root");
    ok &= expect_true(hosts[0].get_autoindex() == false, "default autoindex off");
    ok &= expect_true(hosts[0].get_max_body_size() == 1024 * 1024, "default body size 1m");
    ok &= expect_true(!hosts[0].get_index().empty(), "default index exists");
    return ok;
}

static bool test_location_mapping() {
    const std::string cfg =
        "http { server { listen 8081; location /api { root /srv/api; autoindex on; cgi_extension .py .cgi; upload_location /tmp/up; } } }";

    TokenFactory factory;
    std::string data = cfg;
    factory.extractTokens(data);
    std::vector<IToken*> tokens = factory.getTokens();
    std::vector<__server_row_data> raw = ConfigParser::parse(tokens);
    std::vector<VirtualHost> hosts = build_hosts(raw);

    bool ok = true;
    ok &= expect_true(hosts.size() == 1, "location host count");
    ok &= expect_true(hosts[0].get_locations().size() == 1, "location count");

    const http::core::types::__location& loc = hosts[0].get_locations()[0];
    
    // Debug output
    std::cout << "DEBUG: raw location root='" << raw[0].locations[0].root << "'\n";
    std::cout << "DEBUG: raw location autoindex='" << raw[0].locations[0].autoindex << "'\n";
    std::cout << "DEBUG: location content root='" << loc.content.root << "'\n";
    std::cout << "DEBUG: location content autoindex=" << (loc.content.autoindex ? "true" : "false") << "\n";
    
    ok &= expect_true(loc.route.path == "/api", "location path");
    ok &= expect_true(loc.content.root == "/srv/api", "location root");
    ok &= expect_true(loc.content.autoindex == true, "location autoindex on");
    ok &= expect_true(loc.upload_location == "/tmp/up", "location upload path");
    ok &= expect_true(loc.cgi_extension.size() == 2, "location cgi extensions");
    return ok;
}

static bool test_invalid_body_size_throws() {
    const std::string cfg =
        "http { server { listen 8082; client_max_body_size 10x; } }";

    TokenFactory factory;
    std::string data = cfg;
    factory.extractTokens(data);
    std::vector<IToken*> tokens = factory.getTokens();

    try {
        std::vector<__server_row_data> raw = ConfigParser::parse(tokens);
        (void)build_hosts(raw);
    } catch (const std::exception&) {
        return expect_true(true, "invalid body size throws");
    }
    return expect_true(false, "invalid body size throws");
}

int main() {
    bool ok = true;
    ok &= test_single_virtualhost_defaults();
    ok &= test_location_mapping();
    ok &= test_invalid_body_size_throws();

    if (!ok) {
        std::cerr << "Some server tests failed" << std::endl;
        return 1;
    }
    std::cout << "All server tests passed" << std::endl;
    return 0;
}
