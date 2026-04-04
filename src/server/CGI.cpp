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

		static std::string uint_to_str(uint16_t n) {
			return int_to_str(n);
		}

		static std::pair<std::string, std::string> parse_uri(const std::string& uri) {
			size_t pos = uri.find('?');
			if (pos == std::string::npos)
				return std::make_pair(uri, "");
			return std::make_pair(uri.substr(0, pos), uri.substr(pos + 1));
		}

		static std::string extract_server_name(const std::map<std::string, std::vector<std::string> >& headers) {
			std::map<std::string, std::vector<std::string> >::const_iterator it = headers.find("Host");
			if (it != headers.end() && !it->second.empty()) {
				std::string host = it->second[0];
				size_t colon_pos = host.find(':');
				if (colon_pos != std::string::npos)
					return host.substr(0, colon_pos);
				return host;
			}
			return "localhost";
		}

		CGI::CGI() {}

		Response::_http_response CGI::exec(
			const Request::__http_request& req,
			const std::string& scriptPath,
			const std::string& interpreterPath,
			uint16_t serverPort
		) {
			Response::_http_response res;
			res._status = types::INTERNAL_SERVER_ERROR;
			res._version = "HTTP/1.1";

			// Check if script file exists
			struct stat script_stat;
			if (stat(scriptPath.c_str(), &script_stat) < 0) {
				res._status = types::NOT_FOUND;
				res._body = "CGI script file not found";
				return res;
			}

			int in_pipe[2], out_pipe[2], err_pipe[2];
			if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0 || pipe(err_pipe) < 0) {
				res._body = "Failed to create pipes";
				return res;
			}

			pid_t pid = fork();
			if (pid < 0) {
				close(in_pipe[0]);
				close(in_pipe[1]);
				close(out_pipe[0]);
				close(out_pipe[1]);
				close(err_pipe[0]);
				close(err_pipe[1]);
				res._body = "Failed to fork";
				return res;
			}

			if (pid == 0) {
				// CHILD PROCESS
				dup2(in_pipe[0], STDIN_FILENO);
				dup2(out_pipe[1], STDOUT_FILENO);

				close(in_pipe[1]);
				close(out_pipe[0]);
				close(err_pipe[0]);  // Close read end in child

				// Set close-on-exec for error pipe
				fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);

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
						method_str = "DELETE";
						break;
					default:
						method_str = "GET";
				}

				// Parse URI into path and query
				std::pair<std::string, std::string> uri_parts = parse_uri(req.uri);
				std::string path_info = uri_parts.first;
				std::string query_string = uri_parts.second;

				// Extract SERVER_NAME from Host header
				std::string server_name = extract_server_name(req.headers);

				env.push_back(std::string("REQUEST_METHOD=") + method_str);
				env.push_back(std::string("QUERY_STRING=") + query_string);
				env.push_back(std::string("SCRIPT_FILENAME=") + scriptPath);
				env.push_back(std::string("SCRIPT_NAME=") + path_info);
				env.push_back(std::string("PATH_INFO=") + path_info);
				env.push_back(std::string("PATH_TRANSLATED=") + scriptPath);
				env.push_back(std::string("SERVER_NAME=") + server_name);
				env.push_back(std::string("SERVER_PORT=") + uint_to_str(serverPort));
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

				// Add HTTP_* headers from request headers
				for (std::map<std::string, std::vector<std::string> >::const_iterator hdr = req.headers.begin();
				     hdr != req.headers.end(); ++hdr) {
					// Skip headers already handled
					if (hdr->first == "Content-Type" || hdr->first == "Content-Length" || hdr->first == "Host")
						continue;

					if (!hdr->second.empty()) {
						std::string env_name = "HTTP_";
						for (size_t i = 0; i < hdr->first.size(); ++i) {
							char c = hdr->first[i];
							if (c == '-')
								env_name += '_';
							else if (c >= 'a' && c <= 'z')
								env_name += (c - 'a' + 'A');
							else
								env_name += c;
						}
						env.push_back(env_name + "=" + hdr->second[0]);
					}
				}

				std::vector<char*> envp;
				for (size_t i = 0; i < env.size(); ++i)
					envp.push_back(const_cast<char*>(env[i].c_str()));
				envp.push_back(NULL);

				char* args[3];
				args[0] = const_cast<char*>(interpreterPath.c_str());
				args[1] = const_cast<char*>(scriptPath.c_str());
				args[2] = NULL;

				if (execve(args[0], args, &envp[0]) < 0) {
					// Write error code to error pipe
					int err = errno;
					write(err_pipe[1], &err, sizeof(err));
					close(err_pipe[1]);
				}
				_exit(1);
			} else {
				// PARENT PROCESS
				close(in_pipe[0]);
				close(out_pipe[1]);
				close(err_pipe[1]);  // Close write end in parent

				// Check if child process failed to execute
				char err_code[sizeof(int)];
				ssize_t err_bytes = read(err_pipe[0], err_code, sizeof(int));
				close(err_pipe[0]);

				if (err_bytes == sizeof(int)) {
					// Child exec failed
					int err = *(int*)err_code;
					if (err == ENOENT) {
						res._status = types::NOT_FOUND;
						res._body = "CGI interpreter or script not found";
					} else if (err == EACCES) {
						res._status = types::FORBIDDEN;
						res._body = "Permission denied executing CGI script";
					} else {
						res._status = types::INTERNAL_SERVER_ERROR;
						res._body = "Failed to execute CGI script";
					}
					waitpid(pid, NULL, 0);
					return res;
				}

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
				int wait_count = 0;
				const int MAX_WAIT_ITERATIONS = 300; // 30 seconds with 100ms sleep

				while (wait_count < MAX_WAIT_ITERATIONS) {
					pid_t result = waitpid(pid, &status, WNOHANG);
					if (result == pid) {
						break;
					} else if (result < 0) {
						res._body = "Failed to wait for child process";
						return res;
					}
					usleep(100000); // 100ms
					wait_count++;
				}

				if (wait_count >= MAX_WAIT_ITERATIONS) {
					kill(pid, SIGTERM);
					usleep(500000); // 500ms
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
					res._status = types::GATEWAY_TIMEOUT;
					res._body = "CGI script timeout";
					return res;
				}

				// parse headers from CGI output
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
