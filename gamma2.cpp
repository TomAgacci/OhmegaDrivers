#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <math.h>

#pragma comment(lib, "Comctl32.lib")

static WORD gRamp[3][256];   // current ramp
static float gGamma = 2.2f;  // current gamma
static HWND gStripWnd = NULL;
static HWND gTrackbar = NULL;

void BuildGammaRamp(float gamma, WORD ramp[3][256])
{
    for (int i = 0; i < 256; ++i)
    {
        double x = (double)i / 255.0;
        double y = pow(x, gamma);
        WORD v = (WORD)(y * 65535.0 + 0.5);
        ramp[0][i] = v; // R
        ramp[1][i] = v; // G
        ramp[2][i] = v; // B
    }
}

void ApplyGammaRamp(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    if (hdc)
    {
        SetDeviceGammaRamp(hdc, gRamp);
        ReleaseDC(hwnd, hdc);
    }
}

void PaintStrip(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    int barWidth = width / 256;
    if (barWidth <= 0) barWidth = 1;

    for (int i = 0; i < 256; ++i)
    {
        // ramp is 0..65535, normalize to 0..1, then to 0..255
        double R = (double)gRamp[0][i] / 65535.0;
        double G = (double)gRamp[1][i] / 65535.0;
        double B = (double)gRamp[2][i] / 65535.0;

        BYTE R8 = (BYTE)(R * 255.0 + 0.5);
        BYTE G8 = (BYTE)(G * 255.0 + 0.5);
        BYTE B8 = (BYTE)(B * 255.0 + 0.5);

        HBRUSH hbr = CreateSolidBrush(RGB(R8, G8, B8));
        int x0 = i * barWidth;
        int x1 = x0 + barWidth;
        RECT bar = { x0, 0, x1, height };
        FillRect(hdc, &bar, hbr);
        DeleteObject(hbr);
    }
}

LRESULT CALLBACK StripWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintStrip(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_BAR_CLASSES };
        InitCommonControlsEx(&icc);

        RECT rc;
        GetClientRect(hwnd, &rc);

        int stripHeight = rc.bottom - rc.top - 40;

        gStripWnd = CreateWindowEx(
            0, TEXT("STATIC"), NULL,
            WS_CHILD | WS_VISIBLE,
            0, 0, rc.right, stripHeight,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

        SetWindowLongPtr(gStripWnd, GWLP_WNDPROC, (LONG_PTR)StripWndProc);

        gTrackbar = CreateWindowEx(
            0, TRACKBAR_CLASS, TEXT("Gamma"),
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            10, stripHeight + 5, rc.right - 20, 30,
            hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

        // gamma range: 10..30 -> 1.0..3.0
        SendMessage(gTrackbar, TBM_SETRANGE, TRUE, MAKELPARAM(10, 30));
        SendMessage(gTrackbar, TBM_SETPOS, TRUE, (LPARAM)22); // ~2.2

        // read current ramp
        HDC hdc = GetDC(hwnd);
        if (hdc)
        {
            GetDeviceGammaRamp(hdc, gRamp);
            ReleaseDC(hwnd, hdc);
        }
        else
        {
            BuildGammaRamp(gGamma, gRamp);
        }

        return 0;
    }

    case WM_HSCROLL:
    {
        if ((HWND)lParam == gTrackbar)
        {
            int pos = (int)SendMessage(gTrackbar, TBM_GETPOS, 0, 0);
            gGamma = (float)pos / 10.0f; // 10..30 -> 1.0..3.0

            BuildGammaRamp(gGamma, gRamp);
            ApplyGammaRamp(hwnd);
            InvalidateRect(gStripWnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int stripHeight = rc.bottom - rc.top - 40;
        MoveWindow(gStripWnd, 0, 0, rc.right, stripHeight, TRUE);
        MoveWindow(gTrackbar, 10, stripHeight + 5, rc.right - 20, 30, TRUE);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {0};
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("GammaViewer");

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, TEXT("Global Gamma → Color"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 300,
        NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
