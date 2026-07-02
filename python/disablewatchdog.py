// GammaRamp.h
bool GetGlobalGamma(WORD ramp[3][256]);
bool SetGlobalGammaLinear();
bool IsLinear(const WORD ramp[3][256]);

// Watchdog.cpp
#include "GammaRamp.h"
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic<bool> g_watchdogRunning{false};

void StartWatchdog()
{
    g_watchdogRunning = true;
    std::thread([]{
        WORD ramp[3][256];
        while (g_watchdogRunning)
        {
            if (GetGlobalGamma(ramp) && !IsLinear(ramp))
            {
                SetGlobalGammaLinear();
                // optional: log event, notify UI
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }).detach();
}

void StopWatchdog()
{
    g_watchdogRunning = false;
}
