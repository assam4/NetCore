#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "HttpServer.hpp"
#include "configparser.hpp"
#include "tokenize_factory.hpp"
#include "SignalHandler.hpp"

int main(int argc, char* argv[]) {
	http::core::SignalHandler::setup();
	const char config_path[] = "./config/default.config";
	if (argc > 2) {
		std::cerr << "usage: " << argv[0] << " [config_file]\n";
		return 1;
	}
	std::ifstream config_file;
	if (argc == 2)
		config_file.open(argv[1]);
	else
		config_file.open(config_path);
	if (!config_file.is_open()) {
		std::cerr << "Failed to open config file\n";
		return 1;
	}
	std::stringstream buffer;
	buffer << config_file.rdbuf();
	std::string config_content = buffer.str();
	http::config::lexer::TokenFactory tokenizer;
	tokenizer.extractTokens(config_content);
	std::vector<http::config::lexer::IToken*> tokens = tokenizer.getTokens();
	std::vector<http::config::parser::__server_row_data> raw_config;
	try {
		raw_config = http::config::parser::ConfigParser::parse(tokens);
	} catch (const std::exception& e) {
		std::cerr << "Config parse failed: " << e.what() << "\n";
		return 1;
	}
	http::core::HttpServer server;
	try {
		server.init(raw_config);
	} catch (const std::exception& e) {
		std::cerr << "Server initialization failed: " << e.what() << "\n";
		return 1;
	}
	std::cout << "Webserv listening...\n";
	server.run();
	std::cout << "Webserv stopped\n";
	return 0;
}
