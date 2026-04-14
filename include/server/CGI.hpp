#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <vector>
# include <map>
# include <sstream>
# include <iostream>
# include <cstdlib>
# include <cerrno>
# include <csignal>

# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>

# include "http_request.hpp"
# include "http_response.hpp"

# define BUFFER_SIZE 4096

namespace http {
	namespace core {


		struct PipeSet {
			int in_fd;
			int out_fd;
		};

		class CGI {
			public:
				static Response::_http_response exec(const Request& req, const std::string& scriptPath, const std::string& interpreterPath, uint16_t serverPort);
			private:
				CGI();
				CGI(const CGI&);
				CGI& operator=(const CGI&);

				static std::string method_to_string(types::HttpMethod m);
				static void write_body(int in_fd, const std::string& body);
				static std::string read_output(int out_fd);
				static bool wait_child(pid_t pid, Response::_http_response& res);
				static std::string extract_server_name(const std::map<std::string, std::vector<std::string> >& hdrs);
				static Response::_http_response make_error(types::HttpStatus  status, const std::string& body);
				static std::vector<std::string> build_env(const Request& req, const std::string& scriptPath, uint16_t serverPort);
				static void parse_cgi_output(const std::string& raw, Response::_http_response& res);
				static pid_t spawn_child(const Request& req, const std::string& scriptPath, const std::string& interpreterPath, uint16_t serverPort, PipeSet& pipes, int& out_exec_errno);

		};
	}
}

#endif
