#!/usr/bin/python3
import os
import sys

print("Content-Type: text/html")
print()

print("<html>")
print("<body>")
print("Hello from CGI<br>")
print("Method:", os.environ.get("REQUEST_METHOD"))
print("<br>Query:", os.environ.get("QUERY_STRING"))
print("</body>")
print("</html>")