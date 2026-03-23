#include "configparser.hpp"

namespace http {
    namespace config {
        namespace parser {
            using namespace lexer;

            int         ConfigParser::blockState = Token::UNKNOWN;
            bool        ConfigParser::singleServerMode = true;

            #include <iostream>

            std::vector<__server_row_data> ConfigParser::parse(std::vector<IToken *>& Tokens) {
                std::vector<__server_row_data>  pp;
                blockState = Token::UNKNOWN;
                singleServerMode = true;
                for (std::vector<IToken*>::const_iterator   it = Tokens.begin(); it != Tokens.end(); ++it) {
                    int type = (*it)->getType();
                    switch (type) {
                        case Token::COMMENT:
                            parseComment(it, Tokens);
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
                                parseValue(Tokens, pp, it);
                            break;
                        default:
                            throw std::runtime_error("Syntax error: Unrecognized token detected.\n");
                    }
                }
                if (blockState)
                    throw std::runtime_error("Syntax error: Unclosed braces detected.\n");
                return pp;
            }

            void    ConfigParser::parseComment(std::vector<IToken*>::const_iterator& it , std::vector<IToken *>& tokens) {
                if (it != tokens.begin()) {
                    int prevType = (*(it - 1))->getType();
                    if (!(prevType & Token::OPERATOR) && prevType != Token::COMMENT)
                        throw std::runtime_error("Syntax error: Unexpected comment(#) detected in this context.\n");
                }
            }

            void    ConfigParser::parseKeyword(int type) {
                if (!blockState && singleServerMode && (type == Token::HTTP || type == Token::SERVER)) {
                    blockState |= type;
                    singleServerMode = false;
                }
                else if ((blockState & ((type >> 1) & ~Token::KEYWORD)) && !(blockState & (type & ~Token::KEYWORD)))
                    blockState |= type;
                else
                    throw std::runtime_error("Syntax error: Unexpected token keyword detected in this context.\n");
            }

            void    ConfigParser::parseOpenBrace(std::vector<IToken *>& tokens, std::vector<__server_row_data>& pp, std::vector<IToken*>::const_iterator& it) {
                if (it != tokens.begin()) {
                    int prevType = (*(it - 1))->getType();
                    if (prevType == Token::SERVER)
                        pp.push_back(__server_row_data());
                    else if (prevType == Token::LOCATION)
                        pp.back().locations.push_back(__location_row_data(pp.back()));
                }
                else
                    throw std::runtime_error("Syntax error: Unexpected '{' brace in this context.\n");
            }

            void    ConfigParser::parseCloseBrace(std::vector<IToken*>::const_iterator& it) {
                if (blockState && (((*(it - 1))->getType() & Token::OPERATOR) || ((*(it - 1))->getType() & Token::COMMENT))) {
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
                    throw std::runtime_error("Syntax error: Unexpected '}' brace in this context.\n");
            }

            void    ConfigParser::parseSemicolon(std::vector<IToken*>::const_iterator& it) {
                int prevType = (*(it - 1))->getType();
                if (!(prevType & Token::VALUE) && !(prevType & Token::CLOSE_BRACE))
                    throw std::runtime_error("Syntax error: Unexpected  semicolon(';')  in this context.\n");
            }

            void    ConfigParser::parseProperty(std::vector<IToken*>::const_iterator& it) {
                if ((blockState & Token::SERVER) || (blockState & Token::LOCATION)) {
                    int prevType = (*(it - 1))->getType();
                    if (prevType == Token::OPEN_BRACE || prevType == Token::SEMICOLON || prevType == Token::COMMENT)
                        return;
                }
                else
                    throw std::runtime_error("Syntax error: Unexpected field(property) in this context.\n");
            }

            void    ConfigParser::parseLocationPropery(std::vector<IToken *> &tokens, std::vector<__server_row_data> &pp, std::vector<IToken *>::const_iterator &it) {
                if (it != tokens.begin() && (*(it - 1))->getType() != Token::LOCATION_MODIFIER)
                    pp.back().locations.push_back(__location_row_data());
                __location_row_data& current = pp.back().locations.back();
                if ((*it)->getType() == Token::LOCATION_PATH)
                    current.path = (*it)->getValue();
                else
                    current.modifier = (*it)->getValue();
            }

            void    ConfigParser::parseValue(std::vector<IToken *> &Tokens, std::vector<__server_row_data> &pp, std::vector<IToken *>::const_iterator &it) {
                if (pp.empty() || !blockState)
                    throw std::runtime_error("Syntax error: Unexpected value in this context (outside of blocks).\n");
                int prevType = (*(it - 1))->getType();
                if (!(prevType & Token::PROPERTY))
                    throw std::runtime_error("Syntax error: Unexpected value in this context. (previous is not a property).\n");
                else if ((blockState & (Token::SERVER & ~Token::PROPERTY)) && !(blockState & (1 << 8))
                            && (prevType == Token::LISTEN || prevType == Token::SERVER_NAME))
                    setServerProperty(Tokens, pp.back(), it);
                else if ((blockState & (Token::LOCATION & ~Token::PROPERTY))
                            && (prevType == Token::CGI_EXTENSION || prevType == Token::UPLOAD_LOCATION))
                    setLocationProperty(Tokens, pp.back().locations.back(), it);
                else if (blockState & (1 << 8))  // in LOCATION block
                    setSharedProperty(Tokens, it, static_cast<__shared_row_data*>(&pp.back().locations.back()));
                else if (blockState & (1 << 7))   // in SERVER block
                    setSharedProperty(Tokens, it, static_cast<__shared_row_data*>(&pp.back()));
                else
                    throw std::runtime_error("Syntax error: Unexpected value in this context.\n");
            }

            void    ConfigParser::parse_error_pages(std::vector<IToken *> &tokens, std::vector<IToken *>::const_iterator &it, __shared_row_data *ptr) {
                std::set<std::string>   key;
                for (; (it + 1) != tokens.end() && (*(it + 1))->getType() == Token::VALUE; ++it)
                    key.insert((*it)->getValue());
                ptr->error_pages[key] = (*it)->getValue();
            }

            void    ConfigParser::setSharedProperty(std::vector<IToken *> &tokens, std::vector<IToken *>::const_iterator &it, __shared_row_data *ptr) {
                int type = (*(it - 1))->getType();
                while (true) {
                    switch (type) {
                        case Token::INDEX:
                            ptr->index.insert((*it)->getValue());
                            break;
                        case Token::ERROR_PAGE:
                            if (it + 1 != tokens.end() && (*(it + 1))->getType() == Token::VALUE) {
                                parse_error_pages(tokens, it, ptr);
                            } else {
                                throw std::runtime_error("Syntax error: Unexpected error_page properties value in this context.\n");
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
                                throw std::runtime_error("Syntax error: Unexpected return properties value in this context.\n");
                            }
                            break;
                        case Token::AUTOINDEX:
                            ptr->autoindex = (*it)->getValue();
                            break;
                        default:
                            throw std::runtime_error("Syntax error: Unexpected property in shared property parsing.\n");
                    }
                    if ((it + 1) != tokens.end() && (*(it + 1))->getType() == Token::VALUE)
                        ++it;
                    else
                        break;
                }
            }

            void    ConfigParser::setServerProperty(std::vector<IToken *> &Tokens, __server_row_data& data, std::vector<IToken *>::const_iterator &it) {
                int type = (*(it - 1))->getType();
                for (;(it) != Tokens.end() && (*it)->getType() == Token::VALUE; ++it)
                    switch(type) {
                        case Token::LISTEN:
                                data.listen.insert((*it)->getValue());
                            break;
                        case Token::SERVER_NAME:
                            data.server_name.insert((*it)->getValue());
                            break;
                        default:
                            throw std::runtime_error("Syntex error: Unexpected property in server block.\n");
                    }
            }

            void    ConfigParser::setLocationProperty(std::vector<IToken *> &Tokens, __location_row_data& data, std::vector<IToken *>::const_iterator& it) {
                int type = (*(it - 1))->getType();
                for (; it != Tokens.end() && (*it)->getType() == Token::VALUE; ++it )
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
            }

        }
    }
}
