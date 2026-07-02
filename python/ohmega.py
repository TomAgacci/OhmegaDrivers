import ctypes
import math

user32 = ctypes.WinDLL('user32')
gdi32  = ctypes.WinDLL('gdi32')

HDC   = ctypes.c_void_p
WORD  = ctypes.c_ushort

GetDC = user32.GetDC
GetDC.argtypes = [ctypes.c_void_p]
GetDC.restype  = HDC

ReleaseDC = user32.ReleaseDC
ReleaseDC.argtypes = [ctypes.c_void_p, HDC]
ReleaseDC.restype  = ctypes.c_int

SetDeviceGammaRamp = gdi32.SetDeviceGammaRamp
SetDeviceGammaRamp.argtypes = [HDC, ctypes.c_void_p]
SetDeviceGammaRamp.restype  = ctypes.c_bool

def enable_ohmega_gamma(gamma=2.2):
    hdc = GetDC(None)
    if not hdc:
        raise RuntimeError("GetDC failed")

    RampArray = WORD * (3 * 256)
    ramp = RampArray()

    for i in range(256):
        x = i / 255.0
        v = int(math.pow(x, gamma) * 65535.0 + 0.5)
        if v < 0: v = 0
        if v > 65535: v = 65535

        ramp[0 * 256 + i] = WORD(v)  # R
        ramp[1 * 256 + i] = WORD(v)  # G
        ramp[2 * 256 + i] = WORD(v)  # B

    if not SetDeviceGammaRamp(hdc, ramp):
        ReleaseDC(None, hdc)
        raise RuntimeError("SetDeviceGammaRamp failed")

    ReleaseDC(None, hdc)
    print("Ohmega-style gamma applied (global).")

if __name__ == "__main__":
    enable_ohmega_gamma(2.2)
