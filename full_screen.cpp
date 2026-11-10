/*
cl /nologo /EHsc /std:c++17 /O2 main.cpp ^
   d3d11.lib dxgi.lib user32.lib gdi32.lib ^
   /link /SUBSYSTEM:WINDOWS
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <vector>
#include <cstdint>
#include <cstdlib>

using Microsoft::WRL::ComPtr;

int mouseX = 0;
int mouseY = 0;
bool alternate = false;

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp
) {
    switch (msg) {
    case WM_MOUSEMOVE:
        mouseX = GET_X_LPARAM(lp);
        mouseY = GET_Y_LPARAM(lp);
        return 0;

    case WM_LBUTTONDOWN:
        alternate = !alternate;
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_ESCAPE)
            DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

BOOL CALLBACK EnumMonitor(
    HMONITOR monitor, HDC, LPRECT, LPARAM param
) {
    auto& rects = *reinterpret_cast<std::vector<RECT>*>(param);

    MONITORINFO info = {};
    info.cbSize = sizeof(info);

    if (GetMonitorInfoW(monitor, &info))
        rects.push_back(info.rcMonitor);

    return TRUE;
}

int WINAPI wWinMain(
    HINSTANCE instance, HINSTANCE, PWSTR, int
) {
    SetProcessDPIAware();

    std::vector<RECT> monitors;
    EnumDisplayMonitors(
        nullptr, nullptr, EnumMonitor,
        reinterpret_cast<LPARAM>(&monitors)
    );

    if (monitors.empty())
        return 1;

    // 두 번째 모니터. 없으면 첫 번째.
    RECT r = monitors[monitors.size() > 1 ? 1 : 0];

    int width  = r.right - r.left;
    int height = r.bottom - r.top;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"PixelOutput";

    if (!RegisterClassW(&wc))
        return 1;

    HWND hwnd = CreateWindowExW(
        0, wc.lpszClassName, L"Pixel Output",
        WS_POPUP,
        r.left, r.top, width, height,
        nullptr, nullptr, instance, nullptr
    );

    if (!hwnd)
        return 1;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapchain;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &sd,
        swapchain.GetAddressOf(),
        device.GetAddressOf(),
        nullptr,
        context.GetAddressOf()
    );

    if (FAILED(hr))
        return 2;

    ComPtr<ID3D11Texture2D> backbuffer;

    hr = swapchain->GetBuffer(
        0, IID_PPV_ARGS(backbuffer.GetAddressOf())
    );

    if (FAILED(hr))
        return 3;

    std::vector<uint32_t> pixels(
        size_t(width) * height
    );

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);

    MSG msg = {};
    bool running = true;

    while (running) {
        while (PeekMessageW(
            &msg, nullptr, 0, 0, PM_REMOVE
        )) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!running)
            break;

        // 전체 화면 픽셀 생성
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t red = uint8_t(x * 255 / width);
                uint8_t green = uint8_t(y * 255 / height);
                uint8_t blue = alternate ? 255 : 40;

                // 마우스 위치에 흰색 십자 표시
                if (std::abs(x - mouseX) < 2 ||
                    std::abs(y - mouseY) < 2) {
                    red = green = blue = 255;
                }

                pixels[size_t(y) * width + x] =
                    0xFF000000u |
                    (uint32_t(red) << 16) |
                    (uint32_t(green) << 8) |
                    uint32_t(blue);
            }
        }

        context->UpdateSubresource(
            backbuffer.Get(),
            0, nullptr,
            pixels.data(),
            width * sizeof(uint32_t),
            0
        );

        hr = swapchain->Present(1, 0);
        if (FAILED(hr))
            break;
    }

    context->ClearState();
    return 0;
}
