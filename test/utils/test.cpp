#include "error_page.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <status_code>\n\n";
        std::cerr << "Supported status codes:\n\n";

        std::cerr << "400: Bad Request\n";
        std::cerr << "401: Unauthorized\n";
        std::cerr << "402: Payment Required\n";
        std::cerr << "403: Forbidden\n";
        std::cerr << "404: Not Found\n";
        std::cerr << "405: Method Not Allowed\n";
        std::cerr << "406: Not Acceptable\n";
        std::cerr << "407: Proxy Authentication Required\n";
        std::cerr << "408: Request Timeout\n";
        std::cerr << "409: Conflict\n";
        std::cerr << "410: Gone\n";
        std::cerr << "411: Length Required\n";
        std::cerr << "412: Precondition Failed\n";
        std::cerr << "413: Payload Too Large\n";
        std::cerr << "414: URI Too Long\n";
        std::cerr << "415: Unsupported Media Type\n";
        std::cerr << "416: Range Not Satisfiable\n";

        std::cerr << "500: Internal Server Error\n";
        std::cerr << "501: Not Implemented\n";
        std::cerr << "502: Bad Gateway\n";
        std::cerr << "503: Service Unavailable\n";
        std::cerr << "504: Gateway Timeout\n";
        std::cerr << "505: HTTP Version Not Supported\n";
        std::cerr << "506: Variant Also Negotiates\n";
        std::cerr << "507: Insufficient Storage\n";

        return 1;
    }

        std::stringstream ss(argv[1]);
    int status_code;

    if (!(ss >> status_code))
    {
        std::cerr << "Invalid status code\n";
        return 1;
    }

    std::string result =
        http::core::page_generator::ErrorPageGenerator::generate_page(status_code);

    if (result.empty())
    {
        std::cerr << "Failed to generate page\n";
        return 1;
    }

    std::ofstream out("error_page.html");
    if (!out)
    {
        std::cerr << "Failed to create file\n";
        return 1;
    }

    out << result;
    out.close();

    system("xdg-open error_page.html");

    return 0;
}