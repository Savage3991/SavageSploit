//
// Created by savage on 18.04.2025.
//

#pragma once

#include <memory>

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <functional>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

class render {
    HWND windowhandle;
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* devicecontext;
    ID3D11RenderTargetView* rendertargetview;
    ID3D11Texture2D* backbuffer;

    bool test;
    float dpi_scale;

    using presentfn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    using resizebuffersfn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    presentfn originalpresent;
    resizebuffersfn originalresizebuffers;
    WNDPROC originalwindowproc;

    void renderer();
    bool initializeimgui();
    void cleanupimgui();


    static HRESULT WINAPI present(IDXGISwapChain* InSwapChain, UINT SyncInterval, UINT Flags);
    static HRESULT WINAPI resizebuffers(IDXGISwapChain* InSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    static LRESULT CALLBACK windowprochandler(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam);

    template<typename T>
    void saferelease(T*& ptr) {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

public:
    ImFont* System;
    ImFont* Plex;
    ImFont* Monospace;
    ImFont* Ui;

    static void initialize();
};

inline const auto g_render = std::make_unique<render>();