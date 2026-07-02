import ctypes
import subprocess
import os
import signal

# -----------------------------
# 1. Kill Ohmega EXE if running
# -----------------------------

def kill_ohmega_process():
    try:
        # taskkill silently succeeds even if process isn't running
        subprocess.run(
            ["taskkill", "/IM", "ohmega.exe", "/F"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        print("Ohmega process terminated.")
    except Exception as e:
        print(f"Could not terminate Ohmega: {e}")

# -----------------------------
# 2. Restore normal gamma ramp
# -----------------------------

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

def restore_normal_gamma():
    hdc = GetDC(None)
    if not hdc:
        raise RuntimeError("GetDC failed")

    RampArray = WORD * (3 * 256)
    ramp = RampArray()

    # Linear ramp: 0..65535 evenly
    for i in range(256):
        v = WORD(i * 257)  # 257 * 255 = 65535
        ramp[0 * 256 + i] = v
        ramp[1 * 256 + i] = v
        ramp[2 * 256 + i] = v

    if not SetDeviceGammaRamp(hdc, ramp):
        ReleaseDC(None, hdc)
        raise RuntimeError("SetDeviceGammaRamp failed")

    ReleaseDC(None, hdc)
    print("Windows gamma restored to normal.")

# -----------------------------
# 3. Optional: block Ohmega EXE
# -----------------------------

def block_ohmega_exe(path):
    try:
        # Rename the EXE so it cannot run
        if os.path.exists(path):
            os.rename(path, path + ".disabled")
            print("Ohmega EXE disabled (renamed).")
        else:
            print("Ohmega EXE not found.")
    except Exception as e:
        print(f"Could not disable Ohmega EXE: {e}")

# -----------------------------
# Main
# -----------------------------

if __name__ == "__main__":
    kill_ohmega_process()
    restore_normal_gamma()

    # OPTIONAL: disable the EXE file itself
    ohmega_path = r"C:\path\to\OhmegaDrivers\build\ohmega.exe"
    block_ohmega_exe(ohmega_path)
