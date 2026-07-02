#pragma once
#include <windows.h>

void BuildGammaRamp(float gamma, WORD ramp[3][256]);
void ApplyGammaRamp(WORD ramp[3][256]);
