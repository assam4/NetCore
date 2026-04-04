#include "CGI.hpp"

namespace http {
	namespace core {

		static std::string int_to_str(unsigned long n) {
			if (n == 0) return "0";
			std::string s;
			while (n > 0) {
				char c = '0' + (n % 10);
				s.insert(s.begin(), c);
				n /= 10;
			}
			return s;
		}

		CGI::CGI() {}

		Response::_http_response CGI::exec(
			const Request::__http_request& req,
			const std::string& scriptPath,
			const std::string& interpreterPath
		) {
			Response::_http_response res;
			res._status = types::INTERNAL_SERVER_ERROR;
			res._version = "HTTP/1.1";

			int in_pipe[2], out_pipe[2];
			if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0)
				return res;

			pid_t pid = fork();
			if (pid < 0)
				return res;

			if (pid == 0) {
				// CHILD
				dup2(in_pipe[0], STDIN_FILENO);
				dup2(out_pipe[1], STDOUT_FILENO);

				close(in_pipe[1]);
				close(out_pipe[0]);

				std::vector<std::string> env;

				// Convert HttpMethod enum to string
				std::string method_str;
				switch (req.method) {
					case types::GET:
						method_str = "GET";
						break;
					case types::POST:
						method_str = "POST";
						break;
					case types::DEL:
						method_str = "DEl";
						break;
					default:
						method_str = "GET";
				}

				env.push_back(std::string("REQUEST_METHOD=") + method_str);
				env.push_back(std::string("QUERY_STRING=") + req.uri);
				env.push_back(std::string("SCRIPT_FILENAME=") + scriptPath);
				env.push_back(std::string("CONTENT_LENGTH=") + int_to_str(req.body.size()));

				// Get Content-Type from headers
				std::string content_type_value = "text/plain";
				std::map<std::string, std::vector<std::string> >::const_iterator it = req.headers.find("Content-Type");
				if (it != req.headers.end() && !it->second.empty()) {
					content_type_value = it->second[0];
				}
				env.push_back(std::string("CONTENT_TYPE=") + content_type_value);

				env.push_back("SERVER_PROTOCOL=HTTP/1.1");
				env.push_back("GATEWAY_INTERFACE=CGI/1.1");
				env.push_back("REDIRECT_STATUS=200");

				std::vector<char*> envp;
				for (size_t i = 0; i < env.size(); ++i)
					envp.push_back(const_cast<char*>(env[i].c_str()));
				envp.push_back(NULL);

				char* args[3];
				args[0] = const_cast<char*>(interpreterPath.c_str());
				args[1] = const_cast<char*>(scriptPath.c_str());
				args[2] = NULL;

				execve(args[0], args, &envp[0]);
				_exit(1);
			} else {
				// PARENT
				close(in_pipe[0]);
				close(out_pipe[1]);

				if (!req.body.empty())
					write(in_pipe[1], req.body.c_str(), req.body.size());
				close(in_pipe[1]);

				const size_t BUF = 4096;
				char buffer[BUF];
				std::string output;
				ssize_t r;
				while ((r = read(out_pipe[0], buffer, BUF)) > 0)
					output.append(buffer, r);
				close(out_pipe[0]);

				int status;
				waitpid(pid, &status, 0);

				// parse headers
				size_t pos = output.find("\r\n\r\n");
				std::string headers = pos != std::string::npos ? output.substr(0, pos) : "";
				std::string body = pos != std::string::npos ? output.substr(pos + 4) : output;

				res._body = body;
				res._status = types::OK;

				std::istringstream s(headers);
				std::string line;
				while (std::getline(s, line)) {
					if (line.size() > 0 && line[line.size()-1] == '\r')
						line.erase(line.size()-1);

					size_t p = line.find(": ");
					if (p != std::string::npos) {
						std::string key = line.substr(0, p);
						std::string val = line.substr(p + 2);
						res._headers[key] = val;
					}
					else if (line.find("Status:") == 0) {
						int code = atoi(line.substr(7).c_str());
						if (code >= 100 && code < 600)
							res._status = (types::HttpStatus)code;
					}
				}
			}
			return res;
		}

	}
}