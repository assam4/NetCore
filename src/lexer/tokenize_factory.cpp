#include "tokenize_factory.hpp"

namespace http {
	namespace config {
		namespace lexer {

			namespace TokenTypeNames {
				static std::map<int, std::string> createTokenTypeNames() {
					std::map<int, std::string> m;
					m[Token::KEYWORD] = "KEYWORD";
					m[Token::OPERATOR] = "OPERATOR";
					m[Token::PROPERTY] = "PROPERTY";
					m[Token::VALUE] = "VALUE";
					m[Token::COMMENT] = "COMMENT";
					m[Token::UNKNOWN] = "UNKNOWN";
					m[Token::HTTP] = "HTTP";
					m[Token::SERVER] = "SERVER";
					m[Token::LOCATION] = "LOCATION";
					m[Token::OPEN_BRACE] = "OPEN_BRACE";
					m[Token::CLOSE_BRACE] = "CLOSE_BRACE";
					m[Token::SEMICOLON] = "SEMICOLON";
					m[Token::LISTEN] = "LISTEN";
					m[Token::SERVER_NAME] = "SERVER_NAME";
					m[Token::ROOT] = "ROOT";
					m[Token::INDEX] = "INDEX";
					m[Token::AUTOINDEX] = "AUTOINDEX";
					m[Token::ERROR_PAGE] = "ERROR_PAGE";
					m[Token::ALLOWED_METHODS] = "ALLOWED_METHODS";
					m[Token::CLIENT_MAX_BODY_SIZE] = "CLIENT_MAX_BODY_SIZE";
					m[Token::RETURN] = "RETURN";
					m[Token::CGI_EXTENSION] = "CGI_EXTENSION";
					m[Token::UPLOAD_LOCATION] = "UPLOAD_LOCATION";
					m[Token::LOCATION_PATH] = "LOCATION_PATH";
					m[Token::LOCATION_MODIFIER] = "LOCATION_MODIFIER";
					return m;
				}

				const std::map<int, std::string> names = createTokenTypeNames();

				static std::map<char, Token> createOperations() {
					std::map<char, Token> tokens;

					tokens['{'] = Token::OPEN_BRACE;
					tokens['}'] = Token::CLOSE_BRACE;
					tokens[';'] = Token::SEMICOLON;
					return tokens;
				}

				static const std::map<char, Token> operations = createOperations();

				static std::map<std::string, Token> createKeywords() {
					std::map<std::string, Token> tokens;

					tokens["http"] = Token::HTTP;
					tokens["server"] = Token::SERVER;
					tokens["location"] = Token::LOCATION;
					return tokens;
				}

				static const std::map<std::string, Token> keywords = createKeywords();

				static std::map<std::string, Token> createProperties() {
					std::map<std::string, Token> tokens;

					tokens["listen"] = Token::LISTEN;
					tokens["server_name"] = Token::SERVER_NAME;
					tokens["client_max_body_size"] = Token::CLIENT_MAX_BODY_SIZE;
					tokens["root"] = Token::ROOT;
					tokens["index"] = Token::INDEX;
					tokens["error_page"] = Token::ERROR_PAGE;
					tokens["autoindex"] = Token::AUTOINDEX;
					tokens["return"] = Token::RETURN;
					tokens["allowed_methods"] = Token::ALLOWED_METHODS;
					tokens["cgi_extension"] = Token::CGI_EXTENSION;
					tokens["upload_location"] = Token::UPLOAD_LOCATION;
					return tokens;
				}

				static const std::map<std::string, Token> properties = createProperties();

			}

			static const std::string    spaces = " \t\r\n";
			static const std::string    separators = " \t\r\n{};#";

			void    TokenFactory::ltrim(std::string&  str) {
				size_t pos = str.find_first_not_of(spaces);
				if (pos == std::string::npos)
					str.clear();
				else
					str.erase(0, pos);
			}

			size_t TokenFactory::findTokenEnd(const std::string& str, const std::string& separators) const throw() {
				size_t pos = str.find_first_of(separators);
				if (pos == std::string::npos)
					pos = str.length();
				return pos;
			}

			void    TokenFactory::classifyOperation(std::string& str) {
				if (str.empty())
					return;
				std::map<char, Token>::const_iterator it = TokenTypeNames::operations.find(str[0]);
				if (it != TokenTypeNames::operations.end()) {
					tokens.push_back(new Token(it->second));
					str.erase(0, 1);
				}
			}

			void    TokenFactory::classifyComment(std::string& str) {
				size_t pos = findTokenEnd(str, "\n");
				tokens.push_back(new Token(Token::COMMENT, str.substr(0, pos)));
				str.erase(0, pos);
			}

			bool	TokenFactory::classifyKeyword(std::string& str) {
				size_t  pos = findTokenEnd(str, separators);
				std::map<std::string, Token>::const_iterator it = TokenTypeNames::keywords.find(str.substr(0, pos));
				if (it != TokenTypeNames::keywords.end()) {
					tokens.push_back(new Token(it->second));
					str.erase(0, pos);
					return true;
				}
				else
					return false;
			}

			bool    TokenFactory::classifyProperty(std::string &str) {
				size_t  pos = findTokenEnd(str, separators);
				std::map<std::string, Token>::const_iterator it = TokenTypeNames::properties.find(str.substr(0, pos));
				if (it != TokenTypeNames::properties.end()) {
					tokens.push_back(new Token(it->second));
					str.erase(0, pos);
					return true;
				}
				else
					return false;
			}

			bool    TokenFactory::classifyLocationProperty(std::string& str) {
				size_t  pos = findTokenEnd(str, separators);
				bool    isGet = false;
				if (tokens.empty())
					return false;
				if (tokens.back()->getType() == Token::LOCATION_MODIFIER) {
					tokens.back()->setType(Token::LOCATION_PATH);
					isGet = true;
				}
				else if (tokens.back()->getType() ==  Token::LOCATION) {
					isGet = true;
				}
				if (isGet) {
					tokens.push_back(new Token(Token::LOCATION_MODIFIER, str.substr(0, pos)));
					str.erase(0, pos);
					return true;
				}
				return false;
			}

			void    TokenFactory::classifyWord(std::string& str) {
				if (classifyKeyword(str))
					return ;
				if (classifyProperty(str))
					return ;
				if (classifyLocationProperty(str))
					return ;
				size_t pos = findTokenEnd(str, separators);
				tokens.push_back(new Token(Token::VALUE, str.substr(0, pos)));
				str.erase(0, pos);
			}

			void    TokenFactory::extractTokens(std::string& str) {
				ltrim(str);
				while (!str.empty()) {
					char ch = str[0];
					switch(ch) {
						case '{':
						case '}':
						case ';':
							classifyOperation(str);
							break ;
						case '#':
							classifyComment(str);
							break ;
						default:
							classifyWord(str);
							break ;
					}
					ltrim(str);
				}
			}
			std::vector<IToken*>  TokenFactory::getTokens() const {
				return tokens;
			}
		}
	}
}
