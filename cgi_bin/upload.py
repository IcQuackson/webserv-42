#!/usr/bin/env python
import warnings
# Disable all warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.simplefilter("ignore", category=UserWarning)

#!/usr/bin/env python3
import cgi
import cgitb; cgitb.enable()
import os
import sys
import shutil

UPLOAD_FOLDER = sys.argv[1]
UPLOAD_FILE = "cgi_bin/upload/" + sys.argv[2]

#print(UPLOAD_FOLDER)
#print(UPLOAD_FILE)

def main():
	if not UPLOAD_FILE:
		print("<h1> No file was selected </h1>")
		return

	# Create the uploads folder if it doesn't exist
	if not os.path.exists(UPLOAD_FOLDER):
		os.makedirs(UPLOAD_FOLDER)

	# Save the file
	file_path = os.path.join(UPLOAD_FOLDER, sys.argv[2])
	shutil.copy(UPLOAD_FILE, UPLOAD_FOLDER)

	print(f"File '{UPLOAD_FILE}' successfully uploaded and saved")

if __name__ == '__main__':
	main()