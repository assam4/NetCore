#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include "tokenize_factory.hpp"
#include "configparser.hpp"
#include "virtualhost.hpp"

using namespace http::config::lexer;
using namespace http::config::parser;
using namespace http::core;

// Функция для чтения файла
std::string read_file(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filename);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Функция для печати VirtualHost
void print_virtualhost(const VirtualHost& vh) {
    std::cout << "VirtualHost:" << std::endl;
    std::cout << "  listen:";
    std::set<types::__listen> l = vh.get_listen();
    for (std::set<types::__listen>::const_iterator it = l.begin(); it !=l.end(); ++it)
        std::cout << " [" << it->host << ":" << it->port << ", default=" << (it->default_server ? "true" : "false") << "]";
    std::cout << std::endl;
    std::cout << "  server_name:";
    for (size_t i = 0; i < vh.get_server_name().size(); ++i)
        std::cout << " " << vh.get_server_name()[i];
    std::cout << std::endl;
    std::cout << "  error_pages:";
    for (std::map<uint16_t, std::string>::const_iterator it = vh.get_error_pages().begin(); it != vh.get_error_pages().end(); ++it)
        std::cout << " [" << it->first << "->" << it->second << "]";
    std::cout << std::endl;
    std::cout << "  index:";
    for (std::set<std::string>::const_iterator it = vh.get_index().begin(); it != vh.get_index().end(); ++it)
        std::cout << " " << *it;
    std::cout << std::endl;
    std::cout << "  allowed_methods:";
    for (std::set<std::string>::const_iterator it = vh.get_allowed_methods().begin(); it != vh.get_allowed_methods().end(); ++it)
        std::cout << " " << *it;
    std::cout << std::endl;
    std::cout << "  root: " << vh.get_root() << std::endl;
    std::cout << "  client_max_body_size: " << vh.get_max_body_size() << std::endl;
    std::cout << "  autoindex: " << (vh.get_autoindex() ? "true" : "false") << std::endl;
    std::cout << "  return: code=" << vh.get_redirect_code() << ", path=" << vh.get_redirect_path() << std::endl;
    std::cout << "  locations: " << vh.get_locations().size() << std::endl;
    for (size_t i = 0; i < vh.get_locations().size(); ++i) {
        const types::__location& loc = vh.get_locations()[i];
        std::cout << "    location[" << i << "] path=" << loc.route.path << ", modifier=" << loc.route.modifier << std::endl;
        std::cout << "      root: " << loc.content.root << std::endl;
        std::cout << "      autoindex: " << (loc.content.autoindex ? "true" : "false") << std::endl;
        std::cout << "      upload_location: " << loc.upload_location << std::endl;
        std::cout << "      cgi_extension:";
        for (std::set<std::string>::const_iterator it = loc.cgi_extension.begin(); it != loc.cgi_extension.end(); ++it)
            std::cout << " " << *it;
        std::cout << std::endl;
    }
}

static std::vector<VirtualHost> build_hosts(const std::vector<__server_row_data>& raw) {
    std::vector<VirtualHost> result;
    for (std::vector<__server_row_data>::const_iterator it = raw.begin(); it != raw.end(); ++it)
        result.push_back(VirtualHost::build(*it, result));
    return result;
}


int main() {
    try {
    // Новый тест: читаем конфиг из файла, парсим, сравниваем, печатаем
    const std::string config_path = "../../config/default.config";
    std::string config_data = read_file(config_path);
    TokenFactory factory;
    factory.extractTokens(config_data);
    std::vector<IToken*> tokens = factory.getTokens();
    std::vector<__server_row_data> raw = ConfigParser::parse(tokens);
    std::vector<VirtualHost> hosts = build_hosts(raw);

    std::cout << "\n--- VirtualHost dump ---\n";
    for (size_t i = 0; i < hosts.size(); ++i) {
        std::cout << "VirtualHost #" << i << ":\n";
        print_virtualhost(hosts[i]);
    }

    // Пример сравнения с ожидаемым (эталонным) VirtualHost
    bool ok = true;
    if (!hosts.empty()) {
        uint16_t expected_port = 8080; // замените на нужный
        bool found = false;
        const std::set<http::core::types::__listen>& listens = hosts[0].get_listen();
        for (std::set<http::core::types::__listen>::const_iterator it = listens.begin(); it != listens.end(); ++it) {
            if (it->port == expected_port) found = true;
        }
        ok &= found;
        std::cout << "Check listen port: " << (found ? "PASS" : "FAIL") << std::endl;
        // Добавьте другие проверки по аналогии
    }

    if (!ok) {
        std::cerr << "Some server tests failed" << std::endl;
        return 1;
    }
    std::cout << "All server tests passed" << std::endl;
    if (!ok) {
        std::cerr << "Some server tests failed" << std::endl;
        return 1;
    }
    std::cout << "All server tests passed" << std::endl;
    return 0;
}
catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
}
}
