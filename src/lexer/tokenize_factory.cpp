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
                if (str.empty()) return;
                switch(str[0]) {
                    case '{':
                        tokens.push_back(new Token(Token::OPEN_BRACE)); break ;
                    case '}':
                        tokens.push_back(new Token(Token::CLOSE_BRACE)); break ;
                    case ';':
                        tokens.push_back(new Token(Token::SEMICOLON)); break ;
                    default:
                        return ;
                }
                str.erase(0, 1);
            }

            void    TokenFactory::classifyComment(std::string& str) {
                size_t pos = findTokenEnd(str, "\n");
                tokens.push_back(new Token(Token::COMMENT, str.substr(0, pos)));
                str.erase(0, pos);
            }

            bool    TokenFactory::classifyKeyword(std::string& str) {
                size_t  pos = findTokenEnd(str, separators);
                Token::Type t = Token::UNKNOWN;
                if (str.compare(0, pos, "http") == 0)
                    t = Token::HTTP;
                else if (str.compare(0, pos, "server") == 0)
                    t = Token::SERVER;
                else if (str.compare(0, pos, "location") == 0)
                    t = Token::LOCATION;
                if (t != Token::UNKNOWN) {
                    tokens.push_back(new Token(t));
                    str.erase(0, pos);
                    return true;
                }
                else 
                    return false;
            }

            bool    TokenFactory::classifyProperty(std::string &str)
            {
                size_t  pos = findTokenEnd(str, separators);
                Token::Type t = Token::UNKNOWN;
                if (str.compare(0, pos, "listen") == 0)
                    t = Token::LISTEN;
                else if (str.compare(0, pos, "server_name") == 0)
                    t = Token::SERVER_NAME;
                else if (str.compare(0, pos, "client_max_body_size") == 0)
                    t = Token::CLIENT_MAX_BODY_SIZE;
                else if (str.compare(0, pos, "root") == 0)
                    t = Token::ROOT;
                else if (str.compare(0, pos, "index") == 0)
                    t = Token::INDEX;
                else if (str.compare(0, pos, "error_page") == 0)
                    t = Token::ERROR_PAGE;
                else if (str.compare(0, pos, "autoindex") == 0)
                    t = Token::AUTOINDEX;
                else if (str.compare(0, pos, "return") == 0)
                    t = Token::RETURN;
                else if (str.compare(0, pos, "allowed_methods") == 0)
                    t = Token::ALLOWED_METHODS;
                else if (str.compare(0, pos, "cgi_extension") == 0)
                    t = Token::CGI_EXTENSION;
                else if (str.compare(0, pos, "upload_location") == 0)
                    t = Token::UPLOAD_LOCATION;
                if (t != Token::UNKNOWN) {
                    tokens.push_back(new Token(t));
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
                if (tokens.back()->getType() == Token::LOCATION_PATH) {
                    tokens.back()->setType(Token::LOCATION_MODIFIER);
                    isGet = true;
                }
                else if (tokens.back()->getType() ==  Token::LOCATION) {
                    isGet = true;
                }
                if (isGet) {
                    tokens.push_back(new Token(Token::LOCATION_PATH, str.substr(0, pos)));
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