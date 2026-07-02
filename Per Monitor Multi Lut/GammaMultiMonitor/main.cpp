#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

struct MonitorGamma
{
    DISPLAY_DEVICE dd;
    WORD ramp[3][256];
};

std::vector<MonitorGamma> gMonitors;

void EnumerateMonitors()
{
    gMonitors.clear();
    DISPLAY_DEVICE dd = {};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevices(NULL, i, &dd, 0); ++i)
    {
        if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE)
        {
            MonitorGamma mg{};
            mg.dd = dd;

            HDC hdc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (hdc)
            {
                GetDeviceGammaRamp(hdc, mg.ramp);
                DeleteDC(hdc);
            }
            gMonitors.push_back(mg);
        }
        dd.cb = sizeof(dd);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        EnumerateMonitors();
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        int rows = (int)gMonitors.size();
        if (rows == 0) rows = 1;
        int rowHeight = height / rows;

        for (int m = 0; m < (int)gMonitors.size(); ++m)
        {
            auto& mg = gMonitors[m];
            int top = m * rowHeight;
            int bottom = top + rowHeight;

            int barWidth = width / 256;
            if (barWidth <= 0) barWidth = 1;

            for (int i = 0; i < 256; ++i)
            {
                double R = mg.ramp[0][i] / 65535.0;
                double G = mg.ramp[1][i] / 65535.0;
                double B = mg.ramp[2][i] / 65535.0;

                BYTE R8 = (BYTE)(R * 255.0 + 0.5);
                BYTE G8 = (BYTE)(G * 255.0 + 0.5);
                BYTE B8 = (BYTE)(B * 255.0 + 0.5);

                HBRUSH hbr = CreateSolidBrush(RGB(R8, G8, B8));
                int x0 = i * barWidth;
                int x1 = x0 + barWidth;
                RECT bar = { x0, top, x1, bottom };
                FillRect(hdc, &bar, hbr);
                DeleteObject(hbr);
            }

            // Label with device name
            SetBkMode(hdc, TRANSPARENT);
            TextOut(hdc, 5, top + 5, mg.dd.DeviceName, lstrlen(mg.dd.DeviceName));
        }

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
    wc.lpszClassName = TEXT("GammaMultiMonitor");
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, TEXT("Per‑monitor Gamma Viewer"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 400,
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
