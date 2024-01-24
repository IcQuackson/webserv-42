from datetime import datetime
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
FILENAME = sys.argv[2]
CONTENT = sys.argv[3]
REQUEST_METHOD = os.environ["REQUEST_METHOD"]


def handleGet():
	current_time = get_current_time()
	print(f"Current Time: {current_time}")
	
def get_current_time():
	# Get the current date and time
	current_time = datetime.now()

	# Format the current time as a string
	formatted_time = current_time.strftime("%Y-%m-%d %H:%M:%S")

	return formatted_time

if __name__ == "__main__":
	if REQUEST_METHOD == "GET":
		handleGet()

