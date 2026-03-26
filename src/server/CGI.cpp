#include "CGI.hpp"

namespace http {
	namespace core  {
        CGI::CGI() {};

        std::string CGI::cgiExec(
            const std::string& scriptPath,
            const std::string& interpreterPath,
            const std::string& method,
            const std::string& queryString,
            const std::string& body,
            const std::string& contentType
        ) {
            int in_pipe[2];
            int out_pipe[2];

            if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0)
                return "HTTP/1.1 500 Internal Server Error\r\n\r\nPipe error";

            pid_t pid = fork();

            if (pid < 0)
                return "HTTP/1.1 500 Internal Server Error\r\n\r\nFork error";

            if (pid == 0)
            {
                // ===== CHILD =====

                dup2(in_pipe[0], STDIN_FILENO);
                dup2(out_pipe[1], STDOUT_FILENO);

                close(in_pipe[1]);
                close(out_pipe[0]);

                // --- ENV ---
                std::vector<std::string> env;

                std::ostringstream ss;
                ss << body.size();

                env.push_back("REQUEST_METHOD=" + method);
                env.push_back("QUERY_STRING=" + queryString);
                env.push_back("SCRIPT_FILENAME=" + scriptPath);
                env.push_back("CONTENT_LENGTH=" + ss.str());
                env.push_back("CONTENT_TYPE=" + contentType);
                env.push_back("SERVER_PROTOCOL=HTTP/1.1");
                env.push_back("GATEWAY_INTERFACE=CGI/1.1");

                std::vector<char*> envp;
                for (size_t i = 0; i < env.size(); ++i)
                    envp.push_back(const_cast<char*>(env[i].c_str()));
                envp.push_back(NULL);

                // --- ARGS ---
                char* args[3];
                args[0] = const_cast<char*>(interpreterPath.c_str());
                args[1] = const_cast<char*>(scriptPath.c_str());
                args[2] = NULL;

                execve(args[0], args, &envp[0]);

                // если execve не сработал
                std::exit(1);
            }
            else
            {
                // ===== PARENT =====

                close(in_pipe[0]);
                close(out_pipe[1]);

                if (method == "POST" && !body.empty())
                    write(in_pipe[1], body.c_str(), body.size());

                close(in_pipe[1]);

                char buffer[4096];
                std::string cgiOutput;
                ssize_t bytes;

                while ((bytes = read(out_pipe[0], buffer, sizeof(buffer))) > 0)
                    cgiOutput.append(buffer, bytes);

                close(out_pipe[0]);

                waitpid(pid, NULL, 0);

                return "HTTP/1.1 200 OK\r\n" + cgiOutput;
            }

            return "HTTP/1.1 500 Internal Server Error\r\n\r\nUnknown error";
        }
    }
}