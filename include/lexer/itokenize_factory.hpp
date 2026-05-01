#ifndef ITOKENIZE_FACTORY_HPP
# define ITOKENIZE_FACTORY_HPP

# include <iostream>
# include <vector>
# include <string>

namespace http {
    namespace config {
        namespace lexer {

            /**
            *  @class  IToken
            *  @brief  Abstract product interface for tokens.
            *  @details    Represents a single token with type and value, supporting cloning and printing. 
            */

            class   IToken {
            public:
                virtual ~IToken() {}
                virtual int getType() const throw() = 0;
                virtual const std::string &getValue() const throw() = 0;
                virtual void setType(int) throw() = 0;
                virtual void setValue(const std::string &) = 0;
                virtual IToken *clone() const = 0;
                virtual void print(std::ostream &) const = 0;
                friend std::ostream &operator<<(std::ostream &os, IToken *t) {
                    t->print(os);
                    return os;
                }
            };

            /**
             *  @class  ITokenFactory
             *  @brief  Abstract factory interface for tokenization.
             *  @details   Provides interface for extracting tokens from a string and managing token collection.
             */

            class   ITokenizeFactory {
            public:
                ITokenizeFactory() {}
                ITokenizeFactory(const ITokenizeFactory &oth) {
                    for (std::vector<IToken *>::const_iterator it = oth.tokens.begin(); it != oth.tokens.end(); ++it)
                        tokens.push_back((*it)->clone());
                }
                ITokenizeFactory &operator=(const ITokenizeFactory &oth) {
                    if (this != &oth) {
                        for (std::vector<IToken *>::iterator it = tokens.begin(); it != tokens.end(); ++it)
                            delete *it;
                        tokens.clear();
                        for (std::vector<IToken *>::const_iterator it = oth.tokens.begin(); it != oth.tokens.end(); ++it)
                            tokens.push_back((*it)->clone());
                    }
                    return *this;
                }
                virtual ~ITokenizeFactory() {
                    for (std::vector<IToken *>::iterator it = tokens.begin(); it != tokens.end(); ++it)
                        delete *it;
                }
                virtual void extractTokens(std::string &) = 0;
                virtual std::vector<IToken *> getTokens() const = 0;
            protected:
                std::vector<IToken *> tokens;
            };
        }
    }
}

#endif