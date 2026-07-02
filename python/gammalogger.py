import ctypes
import json
import os

user32 = ctypes.WinDLL('user32')
gdi32  = ctypes.WinDLL('gdi32')

WORD = ctypes.c_ushort
LOG_FILE = os.path.expanduser("~/.ohmega_gamma_log.json")

def log_gamma():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()

    hdc = user32.GetDC(None)
    gdi32.GetDeviceGammaRamp(hdc, ramp)
    user32.ReleaseDC(None, hdc)

    data = {
        "r": [ramp[i] for i in range(256)],
        "g": [ramp[256+i] for i in range(256)],
        "b": [ramp[512+i] for i in range(256)]
    }

    json.dump(data, open(LOG_FILE, "w"))
    print("Gamma logged to", LOG_FILE)

def restore_linear():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    for i in range(256):
        v = WORD(i * 257)
        ramp[i] = ramp[256+i] = ramp[512+i] = v

    hdc = user32.GetDC(None)
    gdi32.SetDeviceGammaRamp(hdc, ramp)
    user32.ReleaseDC(None, hdc)

if __name__ == "__main__":
    log_gamma()
    restore_linear()
