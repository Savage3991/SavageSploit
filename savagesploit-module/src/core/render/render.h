//
// Created by savage on 18.04.2025.
//

#pragma once
#include <memory>


class render {
    HWND windowhandle;
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* devicecontext;
    ID3D11RenderTargetView* rendertargetview;
    ID3D11Texture2D* backbuffer;

    bool open_menu;

    using presentfn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    using resizebuffersfn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    presentfn originalpresent;
    resizebuffersfn originalresizebuffers;
    WNDPROC originalwindowproc;

    void render();
    bool initializeimgui();
    void cleanupimgui();

    HRESULT WINAPI present(IDXGISwapChain* InSwapChain, UINT SyncInterval, UINT Flags);
    HRESULT WINAPI resizebuffers(IDXGISwapChain* InSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    LRESULT CALLBACK windowprochandler(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam);

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

    void initialize();
};

inline const auto g_render = std::make_unique<render>();