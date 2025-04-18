//
// Created by savage on 18.04.2025.
//

#include "render.h"

#include <mutex>

#include "src/rbx/engine/game.h"
#include "src/rbx/taskscheduler/taskscheduler.h"
#include "user_interface/user_interface.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void render::initialize() {
    uintptr_t renderjob = g_taskscheduler->get_job_by_name("RenderJob");
    if (!renderjob)
        return;

    rbx::standard_out::printf(rbx::message_type::message_info, "renderjob: %p", renderjob);

    uintptr_t viewbase = *(uintptr_t*)(renderjob + 0x218);
    uintptr_t deviceaddr = *(uintptr_t*)(viewbase + 0x8);

    g_render->swapchain = *(IDXGISwapChain**)(deviceaddr + 0xA8);
    if (!g_render->swapchain)
        return;

    DXGI_SWAP_CHAIN_DESC swapchaindesc;
    if (FAILED(g_render->swapchain->GetDesc(&swapchaindesc)))
        return;


    g_render->windowhandle = swapchaindesc.OutputWindow;

    if (FAILED(g_render->swapchain->GetDevice(__uuidof(ID3D11Device), (void**)(&g_render->device))))
        return;

    g_render->device->GetImmediateContext(&g_render->devicecontext);

    void** OriginalVTable = *(void***)(g_render->swapchain);
    constexpr size_t VTableSize = 18;
 
    auto ShadowVTable = std::make_unique<void* []>(VTableSize);
    memcpy(ShadowVTable.get(), OriginalVTable, sizeof(void*) * VTableSize);

    g_render->originalpresent = (presentfn)(OriginalVTable[8]);
    ShadowVTable[8] = (void*)(present);

    g_render->originalresizebuffers = (resizebuffersfn)(OriginalVTable[13]);
    ShadowVTable[13] = (void*)(resizebuffers);

    *(void***)(g_render->swapchain) = ShadowVTable.release();
    g_render->originalwindowproc = (WNDPROC)(SetWindowLongPtrW(g_render->windowhandle, GWLP_WNDPROC, (LONG_PTR)(windowprochandler)));
}

LRESULT CALLBACK render::windowprochandler(HWND hwnd, std::uint32_t msg, std::uint64_t wparam, std::int64_t lparam) {
    if (msg == WM_KEYDOWN) {
        if (wparam == VK_INSERT || wparam == VK_DELETE || wparam == VK_END) {
            g_render->test = !g_render->test;
        }
    }
    else if (msg == WM_DPICHANGED) {
        g_render->dpi_scale = LOWORD(wparam) / 96.0f;
    }
    /*else if ( msg == WM_SIZE ) {
        std::uint32_t width = LOWORD( lparam ), height = HIWORD( lparam );
        if ( !is_render_hooked && ( window_width != width || window_height != height ) ) {
            window_width = width, window_height = height;
            window_size_changes.push( true );
        }
    }*/

    if (g_render->test && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    switch (msg) {
    case 522:
    case 513:
    case 533:
    case 514:
    case 134:
    case 516:
    case 517:
    case 258:
    case 257:
    case 132:
    case 127:
    case 255:
    case 523:
    case 524:
    case 793:
        if (g_render->test)
            return true;
        break;
    }

    return CallWindowProc(g_render->originalwindowproc, hwnd, msg, wparam, lparam);
}

HRESULT WINAPI render::resizebuffers(IDXGISwapChain* InSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (g_render) // IF CRASH OR SMTH THEN IT MIGHT BE RELATED TO TS
        g_render->saferelease(g_render->rendertargetview);

    return g_render->originalresizebuffers(InSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

bool render::initializeimgui() {
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    IO.IniFilename = NULL;

    IO.Fonts->AddFontDefault();

    static const ImWchar ranges[] = {
            0x0020, 0x00FF,0x2000, 0x206F,0x3000, 0x30FF,0x31F0, 0x31FF, 0xFF00,
            0xFFEF,0x4e00, 0x9FAF,0x0400, 0x052F,0x2DE0, 0x2DFF,0xA640, 0xA69F, 0
    };

    ImFontConfig Config{ };
    Config.OversampleH = 3;
    Config.OversampleV = 3;

    if (!rendertargetview) {
        ID3D11Texture2D* BackBuffer = nullptr;

        if (SUCCEEDED(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer)))) {
            device->CreateRenderTargetView(BackBuffer, nullptr, &rendertargetview);
            BackBuffer->Release();
        }
        else
            return false;
    }

    ImGui_ImplWin32_Init(windowhandle);
    ImGui_ImplDX11_Init(device, devicecontext);
    return true;
}


HRESULT WINAPI render::present(IDXGISwapChain* InSwapChain, UINT SyncInterval, UINT Flags) {
    static std::once_flag InitFlag;
    std::call_once(InitFlag, []() { if (g_render) g_render->initializeimgui(); });

    if (!g_render) return S_OK;

    if (!g_render->rendertargetview) {
        ID3D11Texture2D* BackBuffer = nullptr;

        if (SUCCEEDED(InSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer)))) {
            g_render->device->CreateRenderTargetView(BackBuffer, nullptr, &g_render->rendertargetview);
            BackBuffer->Release();
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //for (auto obj : Arcadia::drawing_objects) if (obj) obj->draw();

    if (g_render->test) {

        g_user_interface->render();

    }

    ImGui::Render();
    g_render->devicecontext->OMSetRenderTargets(1, &g_render->rendertargetview, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return g_render->originalpresent(InSwapChain, SyncInterval, Flags);
}