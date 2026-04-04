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
# include "http_request.hpp"
# include "http_response.hpp"

# define BUFFER_SIZE 4096

namespace http {
	namespace core  {
        class CGI {
            private:
                CGI ();

            public:
                static Response::_http_response exec(
                    const Request::__http_request& req,
                    const std::string& scriptPath,
                    const std::string& interpreterPath
                );
        };
    };
};

#endif