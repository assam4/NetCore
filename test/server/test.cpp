#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

#include "HttpServer.hpp"
#include "configparser.hpp"   // your parser header
#include "virtualhost.hpp"
#include "config_store.hpp"
#include "tokenize_factory.hpp"

// ─── colour helpers ───────────────────────────────────────────────────────────
#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

static int passed = 0;
static int failed = 0;

std::string to_string(size_t value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

static void ok(const std::string& msg) {
	std::cout << GREEN << "[PASS] " << RESET << msg << "\n";
	++passed;
}
static void fail(const std::string& msg) {
	std::cout << RED << "[FAIL] " << RESET << msg << "\n";
	++failed;
}
static void section(const std::string& msg) {
	std::cout << "\n" << YELLOW << "=== " << msg << " ===" << RESET << "\n";
}

// ─── helper: try to connect to a port ─────────────────────────────────────────
static bool can_connect(uint16_t port) {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return false;

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	bool ok = (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
	::close(fd);
	return ok;
}

// ─── helper: check port is NOT connectable ────────────────────────────────────
static bool port_closed(uint16_t port) {
	return !can_connect(port);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " <config_file>\n";
		return 1;
	}

	// ── 1. Parse config ───────────────────────────────────────────────────────
	section("Config parsing");
	std::vector<http::config::parser::__server_row_data> raw_config;
	try {
		// Read config file into string
		std::ifstream config_file(argv[1]);
		if (!config_file.is_open())
			throw std::runtime_error("Cannot open config file");

		std::stringstream buffer;
		buffer << config_file.rdbuf();
		std::string config_content = buffer.str();
		config_file.close();

		// Tokenize the config content
		http::config::lexer::TokenFactory tokenizer;
		tokenizer.extractTokens(config_content);
		std::vector<http::config::lexer::IToken*> tokens = tokenizer.getTokens();

		// Parse tokens
		raw_config = http::config::parser::ConfigParser::parse(tokens);
		ok("Config file parsed without exception");
	} catch (const std::exception& e) {
		fail(std::string("Config parse threw: ") + e.what());
		return 1;
	}

	if (raw_config.size() >= 2)
		ok("Found 2 server blocks");
	else
		fail("Expected 2 server blocks, got " + to_string(raw_config.size()));

	// ── 2. Build VirtualHosts ─────────────────────────────────────────────────
	section("VirtualHost construction");
	std::vector<http::core::VirtualHost> vhosts;
	try {
		vhosts = http::core::ConfigStore::collect(raw_config);
		ok("ConfigStore::collect() succeeded");
	} catch (const std::exception& e) {
		fail(std::string("collect() threw: ") + e.what());
		return 1;
	}

	// server 1 — check listen ports
	{
		const std::set<http::core::types::__listen>& l = vhosts[0].get_listen();
		// conf has: listen 80 20 40; listen 8080;  → ports 80,20,40,8080
		bool has_80   = false, has_20 = false, has_40 = false, has_8080 = false;
		for (std::set<http::core::types::__listen>::const_iterator it = l.begin();
			 it != l.end(); ++it) {
			if (it->port == 80)   has_80   = true;
			if (it->port == 20)   has_20   = true;
			if (it->port == 40)   has_40   = true;
			if (it->port == 8080) has_8080 = true;
		}
		has_80   ? ok("Server1 listens on port 80")   : fail("Server1 missing port 80");
		has_20   ? ok("Server1 listens on port 20")   : fail("Server1 missing port 20");
		has_40   ? ok("Server1 listens on port 40")   : fail("Server1 missing port 40");
		has_8080 ? ok("Server1 listens on port 8080") : fail("Server1 missing port 8080");
	}

	// server 1 — server_name
	{
		const std::vector<std::string>& names = vhosts[0].get_server_name();
		bool has_example = false, has_www = false;
		for (size_t i = 0; i < names.size(); ++i) {
			if (names[i] == "example.com")     has_example = true;
			if (names[i] == "www.example.com") has_www     = true;
		}
		has_example ? ok("Server1 has server_name example.com")
					: fail("Server1 missing server_name example.com");
		has_www     ? ok("Server1 has server_name www.example.com")
					: fail("Server1 missing server_name www.example.com");
	}

	// server 1 — root
	{
		vhosts[0].get_root() == "/var/www/html"
			? ok("Server1 root = /var/www/html")
			: fail("Server1 root wrong: " + vhosts[0].get_root());
	}

	// server 1 — client_max_body_size (10m = 10*1024*1024)
	{
		size_t expected = 10 * 1024 * 1024;
		vhosts[0].get_max_body_size() == expected
			? ok("Server1 client_max_body_size = 10m")
			: fail("Server1 client_max_body_size wrong");
	}

	// server 1 — locations count
	{
		vhosts[0].get_locations().size() >= 7
			? ok("Server1 has 7+ location blocks")
			: fail("Server1 locations count wrong: "
				   + to_string(vhosts[0].get_locations().size()));
	}

	// server 1 — check specific locations
	{
		const std::vector<http::core::types::__location>& locs = vhosts[0].get_locations();
		bool has_root_loc    = false;
		bool has_api         = false;
		bool has_upload      = false;
		bool has_old_path    = false;
		bool has_static      = false;

		for (size_t i = 0; i < locs.size(); ++i) {
			const std::string& p = locs[i].route.path;
			if (p == "/")         has_root_loc = true;
			if (p == "/api")      has_api      = true;
			if (p == "/upload")   has_upload   = true;
			if (p == "/old_path") has_old_path = true;
			if (p == "/static")   has_static   = true;

			// check upload location field
			if (p == "/upload" && locs[i].upload_location == "/tmp/uploads")
				ok("Upload location path = /tmp/uploads");
			else if (p == "/upload" && locs[i].upload_location != "/tmp/uploads")
				fail("Upload location path wrong: " + locs[i].upload_location);

			// check redirect
			if (p == "/old_path") {
				locs[i].route.code == 301
					? ok("Redirect /old_path code = 301")
					: fail("Redirect /old_path code wrong");
				locs[i].route.new_path == "/new_path"
					? ok("Redirect /old_path target = /new_path")
					: fail("Redirect /old_path target wrong: " + locs[i].route.new_path);
			}

			// check autoindex on for /static
			if (p == "/static") {
				locs[i].content.autoindex
					? ok("/static autoindex = on")
					: fail("/static autoindex should be on");
			}
		}

		has_root_loc ? ok("Location / found")        : fail("Location / missing");
		has_api      ? ok("Location /api found")     : fail("Location /api missing");
		has_upload   ? ok("Location /upload found")  : fail("Location /upload missing");
		has_old_path ? ok("Location /old_path found"): fail("Location /old_path missing");
		has_static   ? ok("Location /static found")  : fail("Location /static missing");
	}

	// server 2 checks
	{
		if (vhosts.size() >= 2) {
			const std::set<http::core::types::__listen>& l = vhosts[1].get_listen();
			bool has_8081 = false;
			for (std::set<http::core::types::__listen>::const_iterator it = l.begin();
				 it != l.end(); ++it)
				if (it->port == 8081) has_8081 = true;

			has_8081 ? ok("Server2 listens on port 8081")
					 : fail("Server2 missing port 8081");

			const std::vector<std::string>& names = vhosts[1].get_server_name();
			bool has_api_name = false;
			for (size_t i = 0; i < names.size(); ++i)
				if (names[i] == "api.example.com") has_api_name = true;
			has_api_name ? ok("Server2 server_name = api.example.com")
						 : fail("Server2 server_name wrong");

			vhosts[1].get_root() == "/var/www/api"
				? ok("Server2 root = /var/www/api")
				: fail("Server2 root wrong: " + vhosts[1].get_root());
		}
	}

	// ── 3. HttpServer::init() — socket binding ────────────────────────────────
	section("Socket binding (HttpServer::init)");
	http::core::HttpServer server;
	try {
		server.init(raw_config);
		ok("HttpServer::init() completed without exception");
	} catch (const std::exception& e) {
		fail(std::string("init() threw: ") + e.what());
		return 1;
	}

	// check ports are actually bound and accepting
	can_connect(80)   ? ok("Port 80 is bound and accepting")
					  : fail("Port 80 not reachable");
	can_connect(8080) ? ok("Port 8080 is bound and accepting")
					  : fail("Port 8080 not reachable");
	can_connect(8081) ? ok("Port 8081 is bound and accepting")
					  : fail("Port 8081 not reachable");

	// port that should NOT be bound
	port_closed(9999) ? ok("Port 9999 correctly not bound")
					  : fail("Port 9999 is open — unexpected");

	// ── 4. find_vhost ────────────────────────────────────────────────────────
	section("VirtualHost matching");

	// exact server_name match
	{
		const http::core::VirtualHost* v = server.find_vhost(8080, "example.com");
		v != NULL
			? ok("find_vhost(8080, example.com) found a vhost")
			: fail("find_vhost(8080, example.com) returned NULL");
		if (v)
			v->get_root() == "/var/www/html"
				? ok("Matched correct vhost (root=/var/www/html)")
				: fail("Matched wrong vhost");
	}

	// www subdomain
	{
		const http::core::VirtualHost* v = server.find_vhost(8080, "www.example.com");
		v != NULL ? ok("find_vhost(8080, www.example.com) found vhost")
				  : fail("find_vhost(8080, www.example.com) returned NULL");
	}

	// server2 domain on its port
	{
		const http::core::VirtualHost* v = server.find_vhost(8081, "api.example.com");
		v != NULL
			? ok("find_vhost(8081, api.example.com) found vhost")
			: fail("find_vhost(8081, api.example.com) returned NULL");
		if (v)
			v->get_root() == "/var/www/api"
				? ok("Matched server2 correctly")
				: fail("Matched wrong server for api.example.com");
	}

	// unknown host → default_server fallback
	{
		const http::core::VirtualHost* v = server.find_vhost(8080, "unknown.com");
		v != NULL ? ok("find_vhost fallback to default_server works")
				  : fail("find_vhost returned NULL for unknown host");
	}

	// wrong port → NULL
	{
		const http::core::VirtualHost* v = server.find_vhost(9999, "example.com");
		v == NULL ? ok("find_vhost(9999, ...) correctly returns NULL")
				  : fail("find_vhost returned vhost for unbound port 9999");
	}

	// ── 5. Summary ────────────────────────────────────────────────────────────
	section("Summary");
	std::cout << GREEN << passed << " passed" << RESET
			  << "  " << RED << failed << " failed" << RESET << "\n\n";

	return failed == 0 ? 0 : 1;
}
