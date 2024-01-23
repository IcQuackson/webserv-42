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
FILENAME = sys.argv[2]
CONTENT = sys.argv[3]

def main():
	# Create the uploads folder if it doesn't exist
	if not os.path.exists(UPLOAD_FOLDER):
		os.makedirs(UPLOAD_FOLDER)

	# Save the file
	file_path = os.path.join(UPLOAD_FOLDER, FILENAME)
	with open(file_path, 'w') as file:
		file.write(CONTENT)

	print(f"File '{FILENAME}' successfully uploaded and saved")

if __name__ == '__main__':
	main()