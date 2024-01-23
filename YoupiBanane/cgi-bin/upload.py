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
CONTENT = sys.argv[2]
FILENAME = sys.argv[3]

print(UPLOAD_FOLDER)
print(CONTENT)

