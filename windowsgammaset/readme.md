How to run it from WSL
WSL cannot directly call Win32 APIs, but it can trigger Windows Python:

From WSL:

Code
cmd.exe /c python restore_color.py
Or if Python is installed in a specific path:

Code
/mnt/c/Users/<you>/AppData/Local/Programs/Python/Python312/python.exe restore_color.py

Run this with:

bash
# In Windows (PowerShell or CMD), not WSL:
python bw_gamma.py
