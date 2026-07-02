#include "Gamma.h"
#include <cmath>

void BuildGammaRamp(float gamma, WORD ramp[3][256])
{
    for (int i = 0; i < 256; ++i)
    {
        double x = (double)i / 255.0;
        double y = pow(x, gamma);
        WORD v = (WORD)(y * 65535.0 + 0.5);
        ramp[0][i] = v;
        ramp[1][i] = v;
        ramp[2][i] = v;
    }
}

void ApplyGammaRamp(WORD ramp[3][256])
{
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        SetDeviceGammaRamp(hdc, ramp);
        ReleaseDC(NULL, hdc);
    }
}
