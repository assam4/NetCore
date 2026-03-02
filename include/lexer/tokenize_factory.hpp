#ifndef TOKENIZE_FACTORY_HPP
# define TOKENIZE_FACTORY_HPP

# include <map> 
# include "itokenize_factory.hpp"

namespace http {
    namespace config {
        namespace lexer {
            
            /**
            *   @class Token
            *   @brief Concrete token implementation for configuration parsing.
            *   @details Represents a single lexical token extracted from a configuration file.
            *          Supports type classification using bitwise flags and provides value storage.
            *          Used as the concrete product in the tokenization factory pattern.
            */      

            namespace TokenTypeNames {
                extern const std::map<int, std::string> names;
            }

            class   Token : public IToken {
                public:
                    enum    Type {
                        UNKNOWN = 0,
                        KEYWORD = 1 << 0,
                        OPERATOR = 1 << 1,
                        PROPERTY = 1 << 2,
                        VALUE = 1 << 3,
                        COMMENT = 1 << 4,
                        HTTP = KEYWORD | (1 << 6),
                        SERVER = KEYWORD | (1 << 7),
                        LOCATION = KEYWORD | (1 << 8),
                        OPEN_BRACE = OPERATOR | (1 << 9),
                        CLOSE_BRACE = OPERATOR | (1 << 10),
                        SEMICOLON = OPERATOR | (1 << 11),
                        LISTEN = PROPERTY | (1 << 12),
                        SERVER_NAME = PROPERTY | (1 << 13),
                        CLIENT_MAX_BODY_SIZE = PROPERTY | (1 << 14),
                        ROOT = PROPERTY | (1 << 15),
                        INDEX = PROPERTY | (1 << 16),
                        ERROR_PAGE = PROPERTY | (1 << 17),
                        AUTOINDEX = PROPERTY | (1 << 18),
                        RETURN = PROPERTY | (1 << 19),
                        ALLOWED_METHODS = PROPERTY | (1 << 20),
                        CGI_EXTENSION = PROPERTY | (1 << 21),
                        UPLOAD_LOCATION = PROPERTY | (1 << 22),
                        LOCATION_PATH = LOCATION | (1 << 23),
                        LOCATION_MODIFIER = LOCATION | (1 << 24)
                    };
                    Token() : type(UNKNOWN) {}
                    Token(Type t, std::string val) : type(t), value(val) {}
                    Token(Type t) : type(t) {}
                    int     getType() const throw() { return static_cast<int>(type); }
                    const std::string &getValue() const throw() { return value; }
                    void    setType(int t) throw() { type = static_cast<Type>(t); }
                    void    setValue(const std::string &val) { value = val; }
                    IToken *clone() const { return new Token(*this); }
                    void    print(std::ostream& os) const { os << "{ Type: " << TokenTypeNames::names.at(type) << ", Value: " << value << " }" << std::endl;}
                private:
                    Type type;
                    std::string value;
            };

            /**
            * @class TokenFactory
            * @brief Concrete factory for tokenizing configuration files.
            * @details Implements token extraction logic, dividing input strings into classified tokens.
            *          Serves as the concrete factory in the tokenization factory pattern.
            */

            class   TokenFactory: public ITokenizeFactory {
                public:
                    void    extractTokens(std::string&);
                    std::vector<IToken*>  getTokens() const;
                private:
                    void    ltrim(std::string&);
                    size_t  findTokenEnd(const std::string&, const std::string&) const throw();
                    void    classifyOperation(std::string&);
                    void    classifyComment(std::string&);
                    bool    classifyKeyword(std::string&);
                    bool    classifyProperty(std::string&);
                    bool    classifyLocationProperty(std::string&);
                    void    classifyWord(std::string&);
            };

        }
    }
}

#endif