import pystray
from pystray import MenuItem
from PIL import Image
import subprocess

def enable():
    subprocess.run(["python", "toggle_ohmega.py"])

def disable():
    subprocess.run(["python", "toggle_ohmega.py"])

def create_icon():
    img = Image.new("RGB", (64, 64), "gray")
    icon = pystray.Icon("OhmegaTray", img, "Ohmega Switcher", menu=pystray.Menu(
        MenuItem("Toggle Ohmega", lambda _: enable()),
        MenuItem("Exit", lambda icon: icon.stop())
    ))
    icon.run()

if __name__ == "__main__":
    create_icon()
