import ctypes
import math
import json
import os

user32 = ctypes.WinDLL('user32')
gdi32  = ctypes.WinDLL('gdi32')

HDC   = ctypes.c_void_p
WORD  = ctypes.c_ushort

GetDC = user32.GetDC
ReleaseDC = user32.ReleaseDC
SetDeviceGammaRamp = gdi32.SetDeviceGammaRamp

STATE_FILE = os.path.expanduser("~/.ohmega_state.json")

def set_gamma(ramp):
    hdc = GetDC(None)
    SetDeviceGammaRamp(hdc, ramp)
    ReleaseDC(None, hdc)

def build_ohmega_gamma(gamma=2.2):
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    for i in range(256):
        x = i / 255.0
        v = int(math.pow(x, gamma) * 65535.0)
        ramp[i] = ramp[256+i] = ramp[512+i] = WORD(v)
    return ramp

def build_linear_gamma():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    for i in range(256):
        v = WORD(i * 257)
        ramp[i] = ramp[256+i] = ramp[512+i] = v
    return ramp

def load_state():
    if not os.path.exists(STATE_FILE):
        return {"enabled": False}
    return json.load(open(STATE_FILE))

def save_state(state):
    json.dump(state, open(STATE_FILE, "w"))

def toggle():
    state = load_state()
    if state["enabled"]:
        print("Disabling Ohmega…")
        set_gamma(build_linear_gamma())
        state["enabled"] = False
    else:
        print("Enabling Ohmega…")
        set_gamma(build_ohmega_gamma())
        state["enabled"] = True
    save_state(state)

if __name__ == "__main__":
    toggle()
