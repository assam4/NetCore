#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include "HttpServer.hpp"
#include "SignalHandler.hpp"
#include "configparser.hpp"
#include "tokenize_factory.hpp"

static const char *DefaultConfigPath = "./config/default.config";

static const char *choose_config_path(int argc, char *argv[]) {
	switch (argc) {
		case 2:
			return argv[1];
		case 1:
			return DefaultConfigPath;
		default:
			throw std::runtime_error("usage: " + std::string(argv[0]) + " [config_file]");
	}
}

static std::string read_config_content(int argc, char *argv[]) {
	try {
		const char *path = choose_config_path(argc, argv);
		std::ifstream config_file(path);
		if (!config_file.is_open())
			throw std::runtime_error("Failed to open config file");
		std::stringstream buffer;
		buffer << config_file.rdbuf();
		return buffer.str();
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Config loading failed: ") + e.what());
	}
}

static std::vector<http::config::lexer::IToken *>   get_tokens(http::config::lexer::ITokenizeFactory& tokenizer, std::string& content) {
	try {
		tokenizer.extractTokens(content);
		return tokenizer.getTokens();
	} catch (const std::exception& e) {
		throw std::runtime_error(std::string("Tokenizing failed: ") + e.what());
	}
}

static std::vector<http::config::parser::__server_row_data> get_parsed_data(std::vector<http::config::lexer::IToken *>& tokens) {
	try {
		return http::config::parser::ConfigParser::parse(tokens);
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Config parse failed: ") + e.what());
	}
}

static void run_server(const std::vector<http::config::parser::__server_row_data>& data) {
	try {
		http::core::HttpServer server;
		server.init(data);
		std::cout << "Webserv listening...\n";
		server.run();
		std::cout << "Webserv stopped\n";
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Server initialization failed: ") + e.what());
	}
}

int main(int argc, char *argv[]) {
	try {
		http::core::SignalHandler::setup();
		std::string config_content = read_config_content(argc, argv);
		http::config::lexer::TokenFactory tokenizer;
		std::vector<http::config::lexer::IToken *> tokens = get_tokens(tokenizer, config_content);
		std::vector<http::config::parser::__server_row_data> raw_config = get_parsed_data(tokens);
		run_server(raw_config);
		return EXIT_SUCCESS;
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "Unknown error...\n";
		return EXIT_FAILURE;
	}
}
