#include "configparser.hpp"

namespace http {
    namespace config {
        namespace parser {
            using namespace lexer;

            int         ConfigParser::blockState = Token::UNKNOWN;
            bool        ConfigParser::singleServerMode = true;

            std::vector<__server_row_data> ConfigParser::parse(std::vector<IToken *>& Tokens) {
                blockState = Token::UNKNOWN;
                singleServerMode = true;
                std::vector<__server_row_data> pp;
                for (const_iter it = Tokens.begin(); it != Tokens.end(); ++it) {
                    int type = (*it)->getType();
                    switch (type) {
                        case Token::COMMENT:
                            break;
                        case Token::HTTP: case Token::SERVER: case Token::LOCATION:
                            parseKeyword(type);
                            break;
                        case Token::OPEN_BRACE:
                            parseOpenBrace(Tokens, pp, it);
                            break;
                        case Token::CLOSE_BRACE:
                            parseCloseBrace(it);
                            break;
                        case Token::SEMICOLON:                                                                                      
                            parseSemicolon(it);
                            break;
                        case Token::INDEX: case Token::ERROR_PAGE: case Token::ALLOWED_METHODS: case Token::ROOT:
                        case Token::RETURN: case Token::AUTOINDEX: case Token::LISTEN: case Token::SERVER_NAME:
                        case Token::CLIENT_MAX_BODY_SIZE: case Token::CGI_EXTENSION: case Token::UPLOAD_LOCATION:
                            parseProperty(it);
                            break;
                        case Token::LOCATION_PATH:
                        case Token::LOCATION_MODIFIER:
                            parseLocationPropery(Tokens, pp, it);
                            break ;
                        case Token::VALUE:
                                parseValue(Tokens, pp, it, type);
                            break;
                        default:
                            throw std::runtime_error("Unexpected Token!");
                    }
                }
                if (blockState)
                    throw std::runtime_error("Unclosed braces!");
                return pp;
            }

            void    ConfigParser::parseKeyword(int type) {
                if (!blockState && singleServerMode && (type == Token::HTTP || type == Token::SERVER)) {
                    blockState |= type;
                    singleServerMode = false;
                }
                else if ((blockState & ((type >> 1) & ~Token::KEYWORD)) && !(blockState & (type & ~Token::KEYWORD)))
                    blockState |= type;
                else
                    throw std::runtime_error("Unexpected keyword block:Keyword is not allowed in this context");
            }

            void    ConfigParser::parseOpenBrace(std::vector<IToken *>& tokens
                                                , std::vector<__server_row_data>& pp
                                                , std::vector<IToken*>::const_iterator& it) {
                if (it == tokens.begin())
                    throw std::runtime_error("Unexpected '{' brace: Open brace is now allowed here");
                if (((*(it - 1))->getType() & Token::KEYWORD)) {
                    if ((*(it - 1))->getType() == Token::SERVER)
                        pp.push_back(__server_row_data());
                    if ((*(it - 1))->getType() == Token::LOCATION)
                        pp.back().locations.push_back(__location_row_data());
                }
                else
                    throw std::runtime_error("Unexpected '{' brace: Open brace is now allowed here");
            }

            void    ConfigParser::parseCloseBrace(std::vector<IToken*>::const_iterator& it) {
                if (blockState && ((*(it - 1))->getType() & Token::OPERATOR)) {
                    if (blockState & (1 << 8))
                        blockState &= ~(1 << 8);
                    else if (blockState & (1 << 7))
                        blockState &= ~(1 << 7);
                    else if (blockState & (1 << 6))
                        blockState &= ~(1 << 6);
                    if (blockState == Token::KEYWORD)
                        blockState = Token::UNKNOWN;
                }
                else
                    throw std::runtime_error("Unexpected '}' brace: Close brace is now allowed here");
            }

            void    ConfigParser::parseSemicolon(std::vector<IToken*>::const_iterator& it) {
                int prevType = (*(it - 1))->getType();
                if (!(prevType & Token::VALUE) && !(prevType & Token::CLOSE_BRACE))
                    throw std::runtime_error("Unexpected ';' operator: Semicolon is now allowed here");
            }

            void    ConfigParser::parseProperty(std::vector<IToken*>::const_iterator& it) {
                int prevType = (*(it - 1))->getType();
                if (!(blockState & Token::SERVER || blockState & Token::LOCATION)
                        || !(prevType & Token::OPEN_BRACE || prevType & Token::SEMICOLON))
                    throw std::runtime_error("Unexpected property here");
            }

            void    ConfigParser::parseLocationPropery(std::vector<IToken *> &tokens
                                                        , std::vector<__server_row_data> &pp
                                                        , std::vector<IToken *>::const_iterator &it) {
                if (it != tokens.begin() && (*(it - 1))->getType() != Token::LOCATION_MODIFIER)
                    pp.back().locations.push_back(__location_row_data());
                __location_row_data& current = pp.back().locations.back();
                if ((*it)->getType() == Token::LOCATION_PATH)
                    current.path = (*it)->getValue();
                else
                    current.modifier = (*it)->getValue(); 
            }

            void    ConfigParser::parseValue(std::vector<IToken *> &Tokens
                                            , std::vector<__server_row_data> &pp
                                            , std::vector<IToken *>::const_iterator &it
                                            , int type) {
                (void)type;
                int prevType = (*(it - 1))->getType();
                if (pp.empty() || !blockState || !(prevType & Token::PROPERTY))
                    throw std::runtime_error("Unexpected value here");
                else if ((blockState & (Token::SERVER & ~Token::PROPERTY))
                            && !(blockState & (1 << 8))
                            && (prevType == Token::LISTEN || prevType == Token::SERVER_NAME))
                    setServerProperty(Tokens, pp.back(), it);               
                else if ((blockState & (Token::LOCATION & ~Token::PROPERTY))
                            && (prevType == Token::CGI_EXTENSION || prevType == Token::UPLOAD_LOCATION))
                    setLocationProperty(Tokens, pp.back().locations.back(), it);
                else if ((blockState & (Token::SERVER & ~Token::PROPERTY))
                            || (blockState & (Token::LOCATION & ~Token::PROPERTY)))
                    setSharedProperty(Tokens, it,
                        (blockState & (Token::SERVER & ~Token::PROPERTY))
                            ? static_cast<__shared_row_data*>(&pp.back())
                            : static_cast<__shared_row_data*>(&pp.back().locations.back()));
                else
                    throw std::runtime_error("Unexpected value here");
            }

            void    ConfigParser::setSharedProperty(std::vector<IToken *> &tokens
                                            , std::vector<IToken *>::const_iterator &it
                                            , __shared_row_data *ptr) {
                if (!ptr) {
                    throw std::runtime_error("Null pointer passed to setSharedProperty");
                }

                int type = (*(it - 1))->getType();
                while (true) {
                    switch (type) {
                        case Token::INDEX:
                            ptr->index.insert((*it)->getValue());
                            break;
                        case Token::ERROR_PAGE:
                            if (it + 1 != tokens.end() && (*(it + 1))->getType() == Token::VALUE) {
                                ptr->error_pages.insert(std::make_pair((*it)->getValue(), (*(it + 1))->getValue()));
                                ++it;
                            } else {
                                throw std::runtime_error("Unexpected value in error_page property");
                            }
                            break;
                        case Token::ALLOWED_METHODS:
                            ptr->allowed_methods.insert((*it)->getValue());
                            break;
                        case Token::CLIENT_MAX_BODY_SIZE:
                            ptr->client_max_body_size = (*it)->getValue();
                            break;
                        case Token::ROOT:
                            ptr->root = (*it)->getValue();
                            break;
                        case Token::RETURN:
                            if (it + 1 != tokens.end() && (*(it + 1))->getType() == Token::VALUE) {
                                ptr->ret_redirection = std::make_pair((*it)->getValue(), (*(it + 1))->getValue());
                                ++it;
                            } else {
                                throw std::runtime_error("Unexpected value in return property");
                            }
                            break;
                        case Token::AUTOINDEX:
                            ptr->autoindex = (*it)->getValue();
                            break;
                        default:
                            throw std::runtime_error("Unexpected error in shared setter!");
                    }
                    if ((it + 1) != tokens.end() && (*(it + 1))->getType() == Token::VALUE)
                        ++it;
                    else 
                        break;
                }
            }

            void    ConfigParser::setServerProperty(std::vector<IToken *> &Tokens
                                                    , __server_row_data& data
                                                    , std::vector<IToken *>::const_iterator &it) {
                if (it == Tokens.begin())
                    throw std::runtime_error("setServerProperty called with invalid iterator");
                int type = (*(it - 1))->getType();
                while (true) {
                    switch(type) {
                        case Token::LISTEN:
                            data.listen.insert((*it)->getValue());
                            break;
                        case Token::SERVER_NAME:
                            data.server_name.insert((*it)->getValue());
                            break;
                        default:
                            throw std::runtime_error("Unexpected Server Property");
                    }
                    if ((it + 1) != Tokens.end() && (*(it + 1))->getType() == Token::VALUE)
                        ++it;
                    else
                        break;
                }
            }

            void    ConfigParser::setLocationProperty(std::vector<IToken *> &Tokens
                                                    , __location_row_data& data
                                                    , std::vector<IToken *>::const_iterator& it) {
                if (it == Tokens.begin())
                    throw std::runtime_error("setLocationProperty called with invalid iterator");
                int type = (*(it - 1))->getType();
                while (true) {
                    switch(type) {
                        case Token::CGI_EXTENSION:
                            data.cgi_extension.insert((*it)->getValue());
                            break;
                        case Token::UPLOAD_LOCATION:
                            data.upload_location = (*it)->getValue();
                            break;
                        default:
                            throw std::runtime_error("Unexpected Location Property");
                    }
                    if ((it + 1) != Tokens.end() && (*(it + 1))->getType() == Token::VALUE)
                        ++it;
                    else
                        break;
                }
            }
        }
    }
}
