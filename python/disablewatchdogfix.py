import ctypes
from ctypes import wintypes

# ---------------------------------------------------------
# Define DISPLAY_DEVICEW manually
# ---------------------------------------------------------
class DISPLAY_DEVICEW(ctypes.Structure):
    _fields_ = [
        ("cb", wintypes.DWORD),
        ("DeviceName", wintypes.WCHAR * 32),
        ("DeviceString", wintypes.WCHAR * 128),
        ("StateFlags", wintypes.DWORD),
        ("DeviceID", wintypes.WCHAR * 128),
        ("DeviceKey", wintypes.WCHAR * 128),
    ]


# ---------------------------------------------------------
# Load Win32 DLLs
# ---------------------------------------------------------
user32 = ctypes.WinDLL("user32")
gdi32  = ctypes.WinDLL("gdi32")
mscms  = ctypes.WinDLL("mscms")

WORD = ctypes.c_ushort


# ---------------------------------------------------------
# Win32 Function Signatures
# ---------------------------------------------------------
GetDC = user32.GetDC
GetDC.argtypes = [ctypes.c_void_p]
GetDC.restype  = ctypes.c_void_p

ReleaseDC = user32.ReleaseDC
ReleaseDC.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
ReleaseDC.restype  = ctypes.c_int

SetDeviceGammaRamp = gdi32.SetDeviceGammaRamp
SetDeviceGammaRamp.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
SetDeviceGammaRamp.restype  = ctypes.c_bool

GetDeviceGammaRamp = gdi32.GetDeviceGammaRamp
GetDeviceGammaRamp.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
GetDeviceGammaRamp.restype  = ctypes.c_bool

EnumDisplayDevicesW = user32.EnumDisplayDevicesW
EnumDisplayDevicesW.argtypes = [
    wintypes.LPCWSTR,
    wintypes.DWORD,
    ctypes.POINTER(DISPLAY_DEVICEW),
    wintypes.DWORD
]
EnumDisplayDevicesW.restype = wintypes.BOOL

# ---------------------------------------------------------
# FIX: Python 3.14 → CreateDCW moved to GDI32
# ---------------------------------------------------------
CreateDCW = gdi32.CreateDCW
CreateDCW.argtypes = [
    wintypes.LPCWSTR,  # driver
    wintypes.LPCWSTR,  # device name
    wintypes.LPCWSTR,  # output
    ctypes.c_void_p    # init data
]
CreateDCW.restype = ctypes.c_void_p

DeleteDC = gdi32.DeleteDC
DeleteDC.argtypes = [ctypes.c_void_p]
DeleteDC.restype  = ctypes.c_bool

WcsSetDefaultColorProfile = mscms.WcsSetDefaultColorProfile


# ---------------------------------------------------------
# Build linear gamma ramp
# ---------------------------------------------------------
def build_linear_ramp():
    RampArray = WORD * (3 * 256)
    ramp = RampArray()
    for i in range(256):
        v = WORD(i * 257)
        ramp[i]       = v
        ramp[256 + i] = v
        ramp[512 + i] = v
    return ramp


# ---------------------------------------------------------
# Restore global gamma
# ---------------------------------------------------------
def restore_global_gamma():
    hdc = GetDC(None)
    if not hdc:
        print("GetDC failed")
        return

    ramp = build_linear_ramp()
    SetDeviceGammaRamp(hdc, ramp)
    ReleaseDC(None, hdc)
    print("Global gamma restored.")


# ---------------------------------------------------------
# Restore per-monitor gamma (CreateDCW FIXED)
# ---------------------------------------------------------
def restore_per_monitor_gamma():
    dd = DISPLAY_DEVICEW()
    dd.cb = ctypes.sizeof(dd)

    ramp = build_linear_ramp()

    for i in range(16):
        if EnumDisplayDevicesW(None, i, ctypes.byref(dd), 0):
            if dd.StateFlags & 0x1:  # active monitor
                hdc = CreateDCW(None, dd.DeviceName, None, None)
                if hdc:
                    SetDeviceGammaRamp(hdc, ramp)
                    DeleteDC(hdc)
                    print(f"Monitor gamma restored: {dd.DeviceName}")


# ---------------------------------------------------------
# Disable ICC/WCS
# ---------------------------------------------------------
def disable_icc():
    WcsSetDefaultColorProfile(
        0,
        None,
        2,
        1,
        "",
        0
    )
    print("ICC/WCS disabled.")


# ---------------------------------------------------------
# Disable watchdog + restore everything
# ---------------------------------------------------------
def disable_watchdog():
    print("Disabling watchdog…")

    restore_global_gamma()
    restore_per_monitor_gamma()
    disable_icc()

    print("Watchdog disabled. Gamma restored.")


# ---------------------------------------------------------
# Main
# ---------------------------------------------------------
if __name__ == "__main__":
    disable_watchdog()
