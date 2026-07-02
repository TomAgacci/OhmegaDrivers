#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mscms.h>
#include <vector>

#pragma comment(lib, "mscms.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        int barWidth = width / 256;
        if (barWidth <= 0) barWidth = 1;

        // Get default system profile
        WCHAR profileName[MAX_PATH] = {};
        DWORD size = sizeof(profileName);
        WcsGetDefaultColorProfile(
            WCS_PROFILE_MANAGEMENT_SCOPE_SYSTEM_WIDE,
            NULL, CPT_ICC, CS_DEVICE, profileName, &size);

        HANDLE hProfile = OpenColorProfileW(
            (PROFILE*)profileName,
            PROFILE_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING);

        // Simple sRGB input profile
        LOGCOLORSPACEW lcs = {};
        lcs.lcsSignature = LCS_SIGNATURE;
        lcs.lcsVersion = 0x400;
        lcs.lcsSize = sizeof(LOGCOLORSPACEW);
        lcs.lcsCSType = LCS_sRGB;
        lcs.lcsIntent = LCS_GM_IMAGES;

        HPROFILE hInput = CreateColorProfile(&lcs);

        HTRANSFORM hXform = CreateColorTransformW(
            hInput, hProfile, NULL, 0);

        struct COLOR_3B { BYTE r, g, b; };
        std::vector<COLOR_3B> src(256), dst(256);

        for (int i = 0; i < 256; ++i)
        {
            src[i].r = (BYTE)i;
            src[i].g = (BYTE)i;
            src[i].b = (BYTE)i;
        }

        TranslateColors(
            hXform,
            src.data(), COLOR_BYTE, 256,
            dst.data(), COLOR_BYTE);

        for (int i = 0; i < 256; ++i)
        {
            HBRUSH hbr = CreateSolidBrush(
                RGB(dst[i].r, dst[i].g, dst[i].b));
            int x0 = i * barWidth;
            int x1 = x0 + barWidth;
            RECT bar = { x0, 0, x1, height };
            FillRect(hdc, &bar, hbr);
            DeleteObject(hbr);
        }

        DeleteColorTransform(hXform);
        CloseColorProfile(hProfile);
        CloseColorProfile(hInput);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = TEXT("GammaWCS");
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, TEXT("Gamma WCS / ICC"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 300,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
