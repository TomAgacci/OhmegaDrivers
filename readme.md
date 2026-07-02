Licensed Under Creative Commons Attribution

Fastest method: Visual Studio (Community Edition, free)
This is the standard Windows workflow.

1. Create a new project
Open Visual Studio

File → New → Project

Choose “Empty Project” under C++

Name it something like GammaViewer

2. Add a source file
Right‑click the Source Files folder

Add → New Item → C++ File (.cpp)

Paste the entire code into that file

3. Enable common controls
The code uses the Windows trackbar, so you need the common controls library:

Visual Studio already links it automatically because of:

c
#pragma comment(lib, "Comctl32.lib")
No extra steps needed.

4. Build
Press Ctrl+Shift+B  
or

Build → Build Solution

5. Run
Press F5  
or

Double‑click the EXE in Debug/ or Release/

Alternative: MinGW‑w64 (GCC)
If you prefer GCC:

1. Install MinGW‑w64
Use MSYS2 or standalone MinGW‑w64.

2. Save the code as gamma.cpp
3. Compile
Run:

Code
g++ gamma.cpp -o gamma.exe -lcomctl32 -lgdi32 -luser32
4. Run
Code
./gamma.exe
Alternative: clang‑cl
If you use LLVM’s Windows toolchain:

Code
clang-cl gamma.cpp /Fe:gamma.exe /link comctl32.lib user32.lib gdi32.lib
What you should see
A window titled Global Gamma → Color

A 256‑pixel color strip showing the current gamma ramp

A slider that changes gamma and updates the strip

The global desktop gamma actually changes when you move the slider
