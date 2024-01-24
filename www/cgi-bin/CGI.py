from datetime import datetime
import warnings
# Disable all warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.simplefilter("ignore", category=UserWarning)

#!/usr/bin/env python3
import cgi
import cgitb; #cgitb.enable()
import os
import sys
import shutil

REQUEST_METHOD = sys.argv[4]


def handleGet():
	# Get the current date and time
	current_time = datetime.now()

	# Format the current time as a string
	formatted_time = current_time.strftime("%Y-%m-%d %H:%M:%S")
	print(f"Current Time: {current_time}")


def handlePost():
	UPLOAD_FOLDER = sys.argv[1]
	FILENAME = sys.argv[2]
	CONTENT = sys.argv[3]

	# Create the uploads folder if it doesn't exist
	if not os.path.exists(UPLOAD_FOLDER):
		os.makedirs(UPLOAD_FOLDER)

	# Save the file
	file_path = os.path.join(UPLOAD_FOLDER, FILENAME)
	with open(file_path, 'w') as file:
		file.write(CONTENT)

	print(f"File '{FILENAME}' successfully uploaded and saved")

if __name__ == "__main__":
	if REQUEST_METHOD == "GET":
		handleGet()
	if REQUEST_METHOD == "POST":
		handlePost()

