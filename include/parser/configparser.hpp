#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <set>
# include <utility>
# include "tokenize_factory.hpp"

namespace http {
    namespace config {
        namespace parser {
            using namespace lexer;

            struct  __shared_row_data {
                std::set<std::string>   index;
                std::map<std::string, std::string>   error_pages;
                std::set<std::string>   allowed_methods;
                std::string client_max_body_size;
                std::string root;
                std::pair<std::string, std::string> ret_redirection;
                std::string autoindex;
            };

            struct  __location_row_data;

            struct  __server_row_data: __shared_row_data {
                std::set<std::string>   listen;
                std::set<std::string>   server_name;
                std::vector<__location_row_data>    locations;
            };

            struct  __location_row_data: __shared_row_data {
                __location_row_data() {}
                __location_row_data(const __server_row_data& serv): __shared_row_data(serv) {}
                std::set<std::string>   cgi_extension;
                std::string upload_location;
                std::string path;
                std::string modifier;
            };
        
            /**
            * @class ConfigParser
            * @brief Parser for nginx-style configuration files
            * @details Implements a syntax analyzer that processes tokenized configuration
            *          data and constructs __server_row_data objects representing server
            *          and location blocks. Uses a state machine approach to track parsing
            *          context (HTTP, SERVER, LOCATION blocks).
            */

            class ConfigParser {
                public:
                    static std::vector<__server_row_data> parse(std::vector<IToken *>& tokens);
                private:
                    typedef typename std::vector<IToken*>::const_iterator const_iter;
                    static void parseKeyword(int);
                    static void parseOpenBrace(std::vector<IToken *>& tokens, std::vector<__server_row_data>&, std::vector<IToken *>::const_iterator&);
                    static void parseCloseBrace(std::vector<IToken *>::const_iterator&);
                    static void parseSemicolon(std::vector<IToken *>::const_iterator&);
                    static void parseProperty(std::vector<IToken *>::const_iterator&);
                    static void parseLocationPropery(std::vector<IToken *>&
                                                    , std::vector<__server_row_data>&
                                                    , std::vector<IToken *>::const_iterator&);
                    static void parseValue(std::vector<IToken *>&
                                            , std::vector<__server_row_data>&
                                            , std::vector<IToken *>::const_iterator& 
                                            , int type);
                    static void setSharedProperty(std::vector<IToken*>&
                                            , std::vector<IToken*>::const_iterator&
                                            , __shared_row_data*);
                    static void setServerProperty(std::vector<IToken*>&
                                            , __server_row_data&
                                            , std::vector<IToken*>::const_iterator&);
                    static void setLocationProperty(std::vector<IToken*>&
                                            , __location_row_data&
                                            , std::vector<IToken*>::const_iterator&);
                    static int  blockState;
                    static bool singleServerMode;
            };
                    }
                }
            }

#endif
