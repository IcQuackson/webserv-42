#!/usr/bin/env python
import cgi
import cgitb

cgitb.enable()

print("Content-Type: text/html")
print()

# Retrieve form data
form = cgi.FieldStorage()

# Check if the file field is present in the form
if "file" in form:
    fileitem = form["file"]

    # Check if the file was uploaded
    if fileitem.filename:
        # Specify the path to save the uploaded file
        upload_path = "./YoupiBanane/upload"
        
        # Save the file
        with open(upload_path + fileitem.filename, 'wb') as f:
            f.write(fileitem.file.read())

        print("<html><body>")
        print("<h2>File successfully uploaded.</h2>")
        print("<p>File name: {0}</p>".format(fileitem.filename))
        print("</body></html>")
    else:
        print("<html><body>")
        print("<h2>No file was uploaded.</h2>")
        print("</body></html>")
else:
    print("<html><body>")
    print("<h2>No 'file' field in the form.</h2>")
    print("</body></html>")