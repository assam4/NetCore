#include "CGI.hpp"

int main()
{
    std::string response = http::core::CGI::cgiExec(
        "./test.py",
        "/usr/bin/python3",
        "GET",
        "name=andranik",
        "",
        "text/plain"
    );

    std::cout << "===== CGI RESPONSE =====\n";
    std::cout << response << std::endl;

    return 0;
}