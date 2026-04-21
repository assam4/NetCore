#include "CGI.hpp"

namespace http {
	namespace core {

		std::string CGI::method_to_string(types::HttpMethod m) {
			switch (m) {
				case types::GET:
					return "GET";
				case types::HEAD:
					return "HEAD";
				case types::POST:
					return "POST";
				case types::DEL:
					return "DELETE";
				default:
					return "GET";
			}
		}

		std::string CGI::extract_server_name(const std::map<std::string, std::vector<std::string> >& hdrs) {
			std::map<std::string, std::vector<std::string> >::const_iterator it = hdrs.find("host");
			if (it != hdrs.end() && !it->second.empty()) {
				const std::string& host = it->second[0];
				size_t colon = host.find(':');
				return (colon != std::string::npos) ? host.substr(0, colon) : host;
			}
			return "localhost";
		}

		Response::_http_response CGI::make_error(types::HttpStatus  status, const std::string& body) {
			Response::_http_response res;
			res._version = "HTTP/1.1";
			res._status  = status;
			res._body    = body;
			return res;
		}

		std::vector<std::string> CGI::build_env(const Request& req, const std::string& scriptPath, uint16_t serverPort) {
			std::string content_type = "text/plain";
			std::map<std::string, std::vector<std::string> >::const_iterator ct = req.headers.header_map.find("content-type");
			if (ct != req.headers.header_map.end() && !ct->second.empty())
				content_type = ct->second[0];
			std::vector<std::string> env;
			env.push_back("REQUEST_METHOD=" + method_to_string(req.start_line.method));
			env.push_back("QUERY_STRING=" + req.start_line.query);
			env.push_back("SCRIPT_FILENAME=" + scriptPath);
			env.push_back("SCRIPT_NAME=" + req.start_line.uri);
			env.push_back("PATH_INFO=" + req.start_line.uri);
			env.push_back("PATH_TRANSLATED=" + scriptPath);
			env.push_back("SERVER_NAME=" + extract_server_name(req.headers.header_map));
			env.push_back("SERVER_PORT=" + to_string(serverPort));
			env.push_back("CONTENT_LENGTH=" + to_string(req.body.content.size()));
			env.push_back("CONTENT_TYPE=" + content_type);
			env.push_back("SERVER_PROTOCOL=" + req.start_line.version);
			env.push_back("GATEWAY_INTERFACE=CGI/1.1");
			env.push_back("REDIRECT_STATUS=200");
			typedef std::map<std::string, std::vector<std::string> >::const_iterator HdrIt;
			for (HdrIt h = req.headers.header_map.begin(); h != req.headers.header_map.end(); ++h) {
				if (h->first == "content-type" || h->first == "content-length" || h->first == "host" || h->second.empty())
					continue;
				std::string name = "HTTP_";
				for (size_t i = 0; i < h->first.size(); ++i) {
					char c = h->first[i];
					if (c == '-')
						name += '_';
					else if (c >= 'a' && c <= 'z')
						name += static_cast<char>(c - 'a' + 'A');
					else
						name += c;
				}
				env.push_back(name + "=" + h->second[0]);
			}
			return env;
		}

		void CGI::parse_cgi_output(const std::string& raw, Response::_http_response& res) {
			std::string hdr_block;
			std::string body;
			size_t sep = raw.find("\r\n\r\n");
			if (sep != std::string::npos) {
				hdr_block = raw.substr(0, sep);
				body = raw.substr(sep + 4);
			} else {
				sep = raw.find("\n\n");
				if (sep != std::string::npos) {
					hdr_block = raw.substr(0, sep);
					body = raw.substr(sep + 2);
				} else {
					body = raw;
				}
			}
			res._body   = body;
			res._status = types::OK;
			std::istringstream stream(hdr_block);
			std::string line;
			while (std::getline(stream, line)) {
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				if (line.compare(0, 7, "Status:") == 0) {
					int code = std::atoi(line.c_str() + 7);
					if (code >= 100 && code < 600)
						res._status = static_cast<types::HttpStatus>(code);
					continue;
				}
				size_t p = line.find(": ");
				if (p != std::string::npos)
					res._headers[line.substr(0, p)] = line.substr(p + 2);
			}
		}

		pid_t CGI::spawn_child(const Request& req, const std::string& scriptPath, const std::string& interpreterPath, uint16_t serverPort, PipeSet& pipes, int& out_exec_errno) {
			int in_pipe[2], out_pipe[2], err_pipe[2];
			if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0 || pipe(err_pipe) < 0)
				return -1;
			pid_t pid = fork();
			if (pid < 0) {
				close(in_pipe[0]);  close(in_pipe[1]);
				close(out_pipe[0]); close(out_pipe[1]);
				close(err_pipe[0]); close(err_pipe[1]);
				return -1;
			}
			if (pid == 0) {
				dup2(in_pipe[0],  STDIN_FILENO);
				dup2(out_pipe[1], STDOUT_FILENO);
				close(in_pipe[0]);  close(in_pipe[1]);
				close(out_pipe[0]); close(out_pipe[1]);
				close(err_pipe[0]);
				fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);
				std::vector<std::string> env_strings = build_env(req, scriptPath, serverPort);
				std::vector<char*> envp;
				for (size_t i = 0; i < env_strings.size(); ++i)
					envp.push_back(const_cast<char*>(env_strings[i].c_str()));
				envp.push_back(NULL);
				char* args[3] = {
					const_cast<char*>(interpreterPath.c_str()),
					const_cast<char*>(scriptPath.c_str()),
					NULL
				};
				execve(args[0], args, &envp[0]);
				int err = errno;
				write(err_pipe[1], &err, sizeof(err));
				close(err_pipe[1]);
				_exit(1);
			}
			close(in_pipe[0]);
			close(out_pipe[1]);
			close(err_pipe[1]);
			out_exec_errno = 0;
			ssize_t n = read(err_pipe[0], &out_exec_errno, sizeof(out_exec_errno));
			close(err_pipe[0]);
			if (n == static_cast<ssize_t>(sizeof(out_exec_errno))) {
				waitpid(pid, NULL, 0);
				close(in_pipe[1]);
				close(out_pipe[0]);
				return -1;
			}
			pipes.in_fd  = in_pipe[1];
			pipes.out_fd = out_pipe[0];
			return pid;
		}

		void CGI::write_body(int in_fd, const std::string& body) {
			if (!body.empty()) {
				signal(SIGPIPE, SIG_IGN);   // child may close stdin before we finish
				write(in_fd, body.c_str(), body.size());
			}
			close(in_fd);
		}

		std::string CGI::read_output(int out_fd) {
			std::string output;
			output.reserve(BUFFER_SIZE);
			char buf[BUFFER_SIZE];
			ssize_t r;
			while ((r = read(out_fd, buf, sizeof(buf))) > 0)
				output.append(buf, static_cast<size_t>(r));
			close(out_fd);
			return output;
		}

		bool CGI::wait_child(pid_t pid, Response::_http_response& res) {
			const int MAX_ITERATIONS = 300;
			const int SLEEP_US = 100000;
			for (int i = 0; i < MAX_ITERATIONS; ++i) {
				int status = 0;
				pid_t result = waitpid(pid, &status, WNOHANG);
				if (result == pid)
					return true;
				usleep(SLEEP_US);
			}
			kill(pid, SIGTERM);
			usleep(500000);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			res._status = types::GATEWAY_TIMEOUT;
			res._body = "CGI script timed out";
			return false;
		}

		CGI::CGI() {}

		Response::_http_response CGI::exec(const Request& req, const std::string& scriptPath, const std::string& interpreterPath, uint16_t serverPort) {
			struct stat st;
			if (stat(scriptPath.c_str(), &st) < 0)
				return make_error(types::NOT_FOUND, "CGI script not found");
			PipeSet pipes = {-1, -1};
			int exec_errno  = 0;
			pid_t pid = spawn_child(req, scriptPath, interpreterPath, serverPort, pipes, exec_errno);
			if (pid < 0) {
				if (exec_errno == ENOENT)
					return make_error(types::NOT_FOUND, "CGI interpreter not found");
				if (exec_errno == EACCES)
					return make_error(types::FORBIDDEN, "Permission denied executing CGI");
				return make_error(types::INTERNAL_SERVER_ERROR, "Failed to spawn CGI process");
			}
			write_body(pipes.in_fd, req.body.content);
			std::string output = read_output(pipes.out_fd);
			Response::_http_response res;
			res._version = "HTTP/1.1";
			if (!wait_child(pid, res))
				return res;
			parse_cgi_output(output, res);
			return res;
		}

	}
}
