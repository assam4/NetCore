#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "tokenize_factory.hpp"

using namespace http::config::lexer;

std::string readFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "✗ Cannot open file: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

bool testTokens(const std::string& name, const std::string& input, const std::vector<int>& expected) {
    TokenFactory tokenizer;
    std::string data = input;
    tokenizer.extractTokens(data);
    std::vector<IToken*> tokens = tokenizer.getTokens();
    std::cout << "\n== " << name << " ==\n";
    bool ok = true;
    if (tokens.size() != expected.size()) {
        std::cout << "FAIL: count mismatch: got " << tokens.size() << ", expected " << expected.size() << std::endl;
        ok = false;
    }
    for (size_t i = 0; i < std::min(tokens.size(), expected.size()); ++i) {
        int t = tokens[i]->getType();
        if (t != expected[i]) {
            std::cout << "FAIL: token " << i << ": got " << t << ", expected " << expected[i] << std::endl;
            ok = false;
        }
    }
    if (ok) std::cout << "SUCCESS\n";
    return ok;
}

int main() {
    // Тест 1: длинная строка с server блоком
    std::string line1 = "server { listen 80; server_name example.com; root /var/www; client_max_body_size 10m; error_page 404 /404.html; autoindex off; allowed_methods GET POST; }";
    int exp1_arr[] = {
        Token::SERVER, Token::OPEN_BRACE,
        Token::LISTEN, Token::VALUE, Token::SEMICOLON,
        Token::SERVER_NAME, Token::VALUE, Token::SEMICOLON,
        Token::ROOT, Token::VALUE, Token::SEMICOLON,
        Token::CLIENT_MAX_BODY_SIZE, Token::VALUE, Token::SEMICOLON,
        Token::ERROR_PAGE, Token::VALUE, Token::VALUE, Token::SEMICOLON,
        Token::AUTOINDEX, Token::VALUE, Token::SEMICOLON,
        Token::ALLOWED_METHODS, Token::VALUE, Token::VALUE, Token::SEMICOLON,
        Token::CLOSE_BRACE
    };
    std::vector<int> exp1(exp1_arr, exp1_arr + sizeof(exp1_arr)/sizeof(exp1_arr[0]));
    testTokens("Long Line 1 (Server)", line1, exp1);

    // Тест 2: длинная строка с location и модификатором
    std::string line2 = "location ~ \\.php$ { cgi_extension .php; root /var/www/html; allowed_methods GET POST; } location /api { root /var/www/api; allowed_methods GET POST PUT; autoindex off; }";
    int exp2_arr[] = {
        Token::LOCATION, Token::LOCATION_MODIFIER, Token::LOCATION_PATH, Token::OPEN_BRACE,
        Token::CGI_EXTENSION, Token::VALUE, Token::SEMICOLON,
        Token::ROOT, Token::VALUE, Token::SEMICOLON,
        Token::ALLOWED_METHODS, Token::VALUE, Token::VALUE, Token::SEMICOLON,
        Token::CLOSE_BRACE,
        Token::LOCATION, Token::LOCATION_PATH, Token::OPEN_BRACE,
        Token::ROOT, Token::VALUE, Token::SEMICOLON,
        Token::ALLOWED_METHODS, Token::VALUE, Token::VALUE, Token::VALUE, Token::SEMICOLON,
        Token::AUTOINDEX, Token::VALUE, Token::SEMICOLON,
        Token::CLOSE_BRACE
    };
    std::vector<int> exp2(exp2_arr, exp2_arr + sizeof(exp2_arr)/sizeof(exp2_arr[0]));
    testTokens("Long Line 2 (Locations)", line2, exp2);

    // Тест 3: файл default.config
    std::string file = readFile("../../config/default.config");
    int exp3_arr[] = {
        65, 514, 129, 514, 16, 4100, 8, 2050, 4100, 8, 2050, 16, 8196, 8, 8, 2050, 16, 16388, 8, 2050, 16, 32772, 8, 2050, 65540, 8, 8, 2050, 16, 131076, 8, 8, 2050, 131076, 8, 8, 8, 8, 8, 2050, 16, 262148, 8, 2050, 16, 1048580, 8, 8, 2050, 16, 16, 257, 8388865, 514, 32772, 8, 2050, 65540, 8, 2050, 262148, 8, 2050, 1048580, 8, 2050, 1026, 16, 257, 8388865, 514, 32772, 8, 2050, 1048580, 8, 8, 8, 2050, 262148, 8, 2050, 1026, 16, 257, 8388865, 514, 1048580, 8, 2050, 4194308, 8, 2050, 16388, 8, 2050, 1026, 16, 257, 16777473, 8388865, 514, 2097156, 8, 2050, 32772, 8, 2050, 1026, 16, 257, 16777473, 8388865, 514, 2097156, 8, 2050, 32772, 8, 2050, 1026, 16, 257, 8388865, 514, 524292, 8, 8, 2050, 1026, 16, 257, 8388865, 514, 32772, 8, 2050, 262148, 8, 2050, 1048580, 8, 2050, 1026, 1026, 16, 129, 514, 4100, 8, 2050, 8196, 8, 2050, 32772, 8, 2050, 16388, 8, 2050, 131076, 8, 8, 2050, 257, 8388865, 514, 1048580, 8, 8, 8, 2050, 1026, 257, 16777473, 8388865, 514, 2097156, 8, 2050, 1026, 1026, 1026
    };
    std::vector<int> exp3(exp3_arr, exp3_arr + sizeof(exp3_arr)/sizeof(exp3_arr[0]));
    testTokens("Default Config File", file, exp3);
    return 0;
}