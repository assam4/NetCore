#include "error_page.hpp"

namespace http {
    namespace core {
        namespace page_generator {

            std::map<int, std::string> ErrorPageGenerator::http_errors;

            void ErrorPageGenerator::fill_errors()
            {
                http_errors[400] = "Bad Request";
                http_errors[401] = "Unauthorized";
                http_errors[402] = "Payment Required";
                http_errors[403] = "Forbidden";
                http_errors[404] = "Not Found";
                http_errors[405] = "Method Not Allowed";
                http_errors[406] = "Not Acceptable";
                http_errors[407] = "Proxy Authentication Required";
                http_errors[408] = "Request Timeout";
                http_errors[409] = "Conflict";
                http_errors[410] = "Gone";
                http_errors[411] = "Length Required";
                http_errors[412] = "Precondition Failed";
                http_errors[413] = "Payload Too Large";
                http_errors[414] = "URI Too Long";
                http_errors[415] = "Unsupported Media Type";
                http_errors[416] = "Range Not Satisfiable";

                http_errors[500] = "Internal Server Error";
                http_errors[501] = "Not Implemented";
                http_errors[502] = "Bad Gateway";
                http_errors[503] = "Service Unavailable";
                http_errors[504] = "Gateway Timeout";
                http_errors[505] = "HTTP Version Not Supported";
                http_errors[506] = "Variant Also Negotiates";
                http_errors[507] = "Insufficient Storage";
            }

            static void replace_values(std::string &data,
                                       const std::string &toSearch,
                                       const std::string &replaceStr)
            {
                size_t pos = 0;

                while ((pos = data.find(toSearch, pos)) != std::string::npos)
                {
                    data.replace(pos, toSearch.length(), replaceStr);
                    pos += replaceStr.length();
                }
            }

            std::string ErrorPageGenerator::generate_page(int status_code)
            {
                if (http_errors.empty())
                    fill_errors();

                std::ifstream file("../../src/utils/example.html");
                if (!file)
                    return "Error";

                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();

                std::stringstream ss;
                ss << status_code;
                std::string statusCodeStr = ss.str();

                std::map<int, std::string>::const_iterator it =
                    http_errors.find(status_code);

                std::string errorName =
                    (it != http_errors.end()) ? it->second : "Unknown Error";

                replace_values(content, "xxx", statusCodeStr);
                replace_values(content, "error_name", errorName);

                return content;
            }

        }
    }
}