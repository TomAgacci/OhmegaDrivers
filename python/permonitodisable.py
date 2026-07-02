import ctypes

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

def disable_per_monitor():
    dd = ctypes.create_unicode_buffer(32)
    display = ctypes.wintypes.DISPLAY_DEVICEW()
    display.cb = ctypes.sizeof(display)

    for i in range(10):
        if user32.EnumDisplayDevicesW(None, i, ctypes.byref(display), 0):
            if display.StateFlags & 0x1:  # active
                hdc = user32.CreateDCW(None, display.DeviceName, None, None)
                ramp = build_linear()
                gdi32.SetDeviceGammaRamp(hdc, ramp)
                user32.DeleteDC(hdc)
                print("Reset:", display.DeviceName)

if __name__ == "__main__":
    disable_per_monitor()
