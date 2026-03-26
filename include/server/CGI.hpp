#ifndef CGI_HPP
# define CGI_HPP

# include <sstream>
# include <string>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <fcntl.h>
# include <cstring>
# include <vector>
# include <string>
# include <iostream>
# include <cstdlib>

# define BUFFER_SIZE 4096

namespace http {
	namespace core  {
        class CGI {
            private:
                CGI ();
            
            public:
                static std::string cgiExec(
                    const std::string& scriptPath,
                    const std::string& interpreterPath,
                    const std::string& method,
                    const std::string& queryString,
                    const std::string& body,
                    const std::string& contentType
                );
        };
    };
};

#endif