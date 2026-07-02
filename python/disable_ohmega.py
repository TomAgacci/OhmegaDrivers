import ctypes

# Load user32 and gdi32
user32 = ctypes.WinDLL('user32')
gdi32  = ctypes.WinDLL('gdi32')

# Types
HDC   = ctypes.c_void_p
WORD  = ctypes.c_ushort

# GetDC(NULL)
GetDC = user32.GetDC
GetDC.argtypes = [ctypes.c_void_p]
GetDC.restype  = HDC

# ReleaseDC(NULL, hdc)
ReleaseDC = user32.ReleaseDC
ReleaseDC.argtypes = [ctypes.c_void_p, HDC]
ReleaseDC.restype  = ctypes.c_int

# BOOL SetDeviceGammaRamp(HDC, LPVOID)
SetDeviceGammaRamp = gdi32.SetDeviceGammaRamp
SetDeviceGammaRamp.argtypes = [HDC, ctypes.c_void_p]
SetDeviceGammaRamp.restype  = ctypes.c_bool

def disable_ohmega():
    hdc = GetDC(None)
    if not hdc:
        raise RuntimeError("GetDC failed")

    # ramp[3][256] of WORD
    RampArray = WORD * (3 * 256)
    ramp = RampArray()

    # Linear ramp: 0..65535 evenly
    for i in range(256):
        v = WORD(i * 257)  # 257 * 255 = 65535
        ramp[0 * 256 + i] = v  # R
        ramp[1 * 256 + i] = v  # G
        ramp[2 * 256 + i] = v  # B

    if not SetDeviceGammaRamp(hdc, ramp):
        ReleaseDC(None, hdc)
        raise RuntimeError("SetDeviceGammaRamp failed")

    ReleaseDC(None, hdc)
    print("Ohmega disabled — Windows color restored.")

if __name__ == "__main__":
    disable_ohmega()
