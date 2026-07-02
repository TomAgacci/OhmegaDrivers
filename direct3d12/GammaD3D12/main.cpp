#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include <cmath>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;

static const UINT FrameCount = 2;
WORD gRamp[3][256];

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // Build a simple gamma ramp
    BuildGammaRamp(2.2f, gRamp);

    // Create window
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = TEXT("GammaD3D12");
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, TEXT("Gamma D3D12"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 300,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nCmdShow);

    // DXGI factory, device, queue
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter1> adapter;
    factory->EnumAdapters1(0, &adapter);

    ComPtr<ID3D12Device> device;
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    ComPtr<ID3D12CommandQueue> queue;
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue));
    }

    // Swapchain
    ComPtr<IDXGISwapChain1> swapChain1;
    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.BufferCount = FrameCount;
        desc.Width = 800;
        desc.Height = 300;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.SampleDesc.Count = 1;

        factory->CreateSwapChainForHwnd(
            queue.Get(), hwnd, &desc, nullptr, nullptr, &swapChain1);
    }

    ComPtr<IDXGISwapChain3> swapChain;
    swapChain1.As(&swapChain);
    UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

    // RTV heap
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = FrameCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap));
    }
    UINT rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Render targets
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < FrameCount; ++i)
        {
            swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
            device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, handle);
            handle.Offset(1, rtvSize);
        }
    }

    // Command allocator & list
    ComPtr<ID3D12CommandAllocator> allocator;
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

    ComPtr<ID3D12GraphicsCommandList> cmdList;
    device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));
    cmdList->Close();

    // Fence
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue = 0;
    device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    auto WaitForGPU = [&]() {
        fenceValue++;
        queue->Signal(fence.Get(), fenceValue);
        if (fence->GetCompletedValue() < fenceValue)
        {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    };

    // Main loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        allocator->Reset();
        cmdList->Reset(allocator.Get(), nullptr);

        // Clear to a simple gradient based on gamma ramp
        float clearColor[4] = { 0, 0, 0, 1 };
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmdList->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            rtvHeap->GetCPUDescriptorHandleForHeapStart());
        rtvHandle.ptr += frameIndex * rtvSize;

        cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        // Use mid‑level gamma ramp entry as clear color
        int idx = 128;
        float R = gRamp[0][idx] / 65535.0f;
        float G = gRamp[1][idx] / 65535.0f;
        float B = gRamp[2][idx] / 65535.0f;
        clearColor[0] = R;
        clearColor[1] = G;
        clearColor[2] = B;

        cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        cmdList->ResourceBarrier(1, &barrier);

        cmdList->Close();
        ID3D12CommandList* lists[] = { cmdList.Get() };
        queue->ExecuteCommandLists(1, lists);

        swapChain->Present(1, 0);
        WaitForGPU();
        frameIndex = swapChain->GetCurrentBackBufferIndex();
    }

    WaitForGPU();
    CloseHandle(fenceEvent);
    return (int)msg.wParam;
}
