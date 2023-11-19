#!/usr/bin/env python

import os

print("Content-type: text/html\n")  # Content-type header indicating HTML output
print("<html><body>")
print("<h1>CGI Script Environment Variables</h1>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}:</strong> {value}</li>")
print("</ul>")
print("</body></html>")
