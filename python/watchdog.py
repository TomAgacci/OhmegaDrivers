import ctypes
import time

user32 = ctypes.WinDLL('user32')
gdi32  = ctypes.WinDLL('gdi32')

WORD = ctypes.c_ushort

def build_linear():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    for i in range(256):
        v = WORD(i * 257)
        ramp[i] = ramp[256+i] = ramp[512+i] = v
    return ramp

def read_gamma():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    hdc = user32.GetDC(None)
    gdi32.GetDeviceGammaRamp(hdc, ramp)
    user32.ReleaseDC(None, hdc)
    return [ramp[i] for i in range(256)]

def watchdog():
    linear = [i * 257 for i in range(256)]
    while True:
        current = read_gamma()
        if current != linear:
            print("Ohmega detected — restoring linear gamma.")
            hdc = user32.GetDC(None)
            gdi32.SetDeviceGammaRamp(hdc, build_linear())
            user32.ReleaseDC(None, hdc)
        time.sleep(0.5)

if __name__ == "__main__":
    watchdog()
