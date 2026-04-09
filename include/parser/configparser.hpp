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

            /**
            * @struct __shared_row_data
            * @brief Raw configuration data common to both server and location blocks.
            * @details Stores unparsed configuration directives (strings, sets) that are shared
            *          between server and location contexts. Acts as base class for block-specific
            *          raw data structures. Values are extracted directly from tokens before validation.
            */
            struct  __shared_row_data {
                std::set<std::string>   index;
                std::map<std::set<std::string>, std::string>   error_pages;
                std::set<std::string>   allowed_methods;
                std::string client_max_body_size;
                std::string root;
                std::pair<std::string, std::string> ret_redirection;
                std::string autoindex;
            }; 

            /**
            * @struct __location_row_data
            * @brief Forward declaration for raw location block data.
            * @details Declared early to support self-referential server/location structures.
            *          Full definition appears below with all location-specific directives.
            */
            struct  __location_row_data;

            /**
            * @struct __server_row_data
            * @brief Raw parsed data representing a single server block from configuration file.
            * @details Inherits shared properties (root, index, error_pages) and adds server-specific
            *          directives (listen addresses, server names) and child location blocks.
            *          Intermediate representation between raw tokens and typed VirtualHost objects.
            */
            struct  __server_row_data: __shared_row_data {
                std::set<std::string>   listen;
                std::set<std::string>   server_name;
                std::vector<__location_row_data>    locations;
            };

            /**
            * @struct __location_row_data
            * @brief Raw parsed data representing a single location block within a server block.
            * @details Inherits shared properties and adds location-specific directives (path,
            *          modifier, CGI extensions, upload location). Created by parser during
            *          server block processing. Converted to typed __location during VirtualHost build.
            */
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
                    static std::vector<__server_row_data> parse(std::vector<IToken *>&);
                private:
                    static void parseComment(std::vector<IToken*>::const_iterator&, std::vector<IToken *>&);
                    static void parseKeyword(int);
                    static void parseOpenBrace(std::vector<IToken *>&, std::vector<__server_row_data>&, std::vector<IToken *>::const_iterator&);
                    static void parseCloseBrace(std::vector<IToken *>::const_iterator&);
                    static void parseSemicolon(std::vector<IToken *>::const_iterator&);
                    static void parseProperty(std::vector<IToken *>::const_iterator&);

                    static void parse_error_pages(std::vector<IToken *>&
                                            , std::vector<IToken *>::const_iterator&
                                            , __shared_row_data*);
                    static void parseLocationPropery(std::vector<IToken *>&
                                                    , std::vector<__server_row_data>&
                                                    , std::vector<IToken *>::const_iterator&);
                    static void parseValue(std::vector<IToken *>&
                                            , std::vector<__server_row_data>&
                                            , std::vector<IToken *>::const_iterator&);
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
