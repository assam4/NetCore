#!/usr/bin/python3
import os
import sys

# Print proper CGI headers
print("Content-Type: text/html")
print()

# Read POST body data if present
post_data = ""
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
if content_length > 0:
    post_data = sys.stdin.read(content_length)

# Print actual CGI response body
print("<html>")
print("<head><title>CGI Test Response</title></head>")
print("<body>")
print("<h1>CGI Script Response</h1>")
print("<hr>")

print("<h2>Request Information</h2>")
print("<table border='1'>")
print("<tr><td><b>REQUEST_METHOD</b></td><td>{}</td></tr>".format(os.environ.get("REQUEST_METHOD", "N/A")))
print("<tr><td><b>QUERY_STRING</b></td><td>{}</td></tr>".format(os.environ.get("QUERY_STRING", "(empty)")))
print("<tr><td><b>SCRIPT_FILENAME</b></td><td>{}</td></tr>".format(os.environ.get("SCRIPT_FILENAME", "N/A")))
print("<tr><td><b>SCRIPT_NAME</b></td><td>{}</td></tr>".format(os.environ.get("SCRIPT_NAME", "N/A")))
print("<tr><td><b>PATH_INFO</b></td><td>{}</td></tr>".format(os.environ.get("PATH_INFO", "N/A")))
print("</table>")

print("<h2>Server Information</h2>")
print("<table border='1'>")
print("<tr><td><b>SERVER_NAME</b></td><td>{}</td></tr>".format(os.environ.get("SERVER_NAME", "N/A")))
print("<tr><td><b>SERVER_PORT</b></td><td>{}</td></tr>".format(os.environ.get("SERVER_PORT", "N/A")))
print("<tr><td><b>SERVER_PROTOCOL</b></td><td>{}</td></tr>".format(os.environ.get("SERVER_PROTOCOL", "N/A")))
print("<tr><td><b>GATEWAY_INTERFACE</b></td><td>{}</td></tr>".format(os.environ.get("GATEWAY_INTERFACE", "N/A")))
print("</table>")

print("<h2>Content Information</h2>")
print("<table border='1'>")
print("<tr><td><b>CONTENT_TYPE</b></td><td>{}</td></tr>".format(os.environ.get("CONTENT_TYPE", "N/A")))
print("<tr><td><b>CONTENT_LENGTH</b></td><td>{}</td></tr>".format(os.environ.get("CONTENT_LENGTH", "0")))
print("</table>")

# Display POST body if present
if post_data:
    print("<h2>Request Body (POST Data)</h2>")
    print("<pre style='background-color: #f0f0f0; padding: 10px; border-radius: 5px;'>")
    # Escape HTML special characters
    post_data_escaped = post_data.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    print(post_data_escaped)
    print("</pre>")

print("<h2>HTTP Headers</h2>")
print("<table border='1'>")
for key in sorted(os.environ.keys()):
    if key.startswith("HTTP_"):
        print("<tr><td><b>{}</b></td><td>{}</td></tr>".format(key, os.environ[key]))
print("</table>")

print("<hr>")
print("<p>Response generated at: {}:{}</p>".format(os.environ.get("SERVER_NAME", "localhost"), os.environ.get("SERVER_PORT", "8080")))
print("</body>")
print("</html>")
