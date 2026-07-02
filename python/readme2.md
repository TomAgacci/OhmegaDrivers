How to run it from WSL
WSL cannot directly call Win32 APIs, but it can trigger Windows Python:

Code
cmd.exe /c python disable_ohmega.py
Or if Python is installed in a specific path:

Code
/mnt/c/Users/<you>/AppData/Local/Programs/Python/Python312/python.exe disable_ohmega.py
