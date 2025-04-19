#include "render.h"
#include "../../rbx/taskscheduler/taskscheduler.h"
#include "user_interface/user_interface.h"

HWND renderer::windowhandle = nullptr;
IDXGISwapChain* renderer::swapchain = nullptr;
ID3D11Device* renderer::device = nullptr;
ID3D11DeviceContext* renderer::devicecontext = nullptr;
ID3D11RenderTargetView* renderer::rendertargetview = nullptr;
ID3D11Texture2D* renderer::backbuffer = nullptr;

renderer::presentfn renderer::originalpresent = nullptr;
renderer::resizebuffersfn renderer::originalresizebuffers = nullptr;
WNDPROC renderer::originalwindowproc = nullptr;

float renderer::dpi_scale = 0;
bool renderer::test = false;

ImFont* renderer::System = nullptr;
ImFont* renderer::Plex = nullptr;
ImFont* renderer::Monospace = nullptr;
ImFont* renderer::Ui = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void renderer::initialize() {
    uintptr_t renderjob = g_taskscheduler->get_job_by_name("RenderJob");
    if (!renderjob)
        return;

    uintptr_t viewbase = *(uintptr_t*)(renderjob + 0x218);
    uintptr_t deviceaddr = *(uintptr_t*)(viewbase + 0x8);

    swapchain = *(IDXGISwapChain**)(deviceaddr + 0xC8);
    if (!swapchain)
        return;

    DXGI_SWAP_CHAIN_DESC swapchaindesc;
    if (FAILED(swapchain->GetDesc(&swapchaindesc)))
        return;


    windowhandle = swapchaindesc.OutputWindow;

    if (FAILED(swapchain->GetDevice(__uuidof(ID3D11Device), (void**)(&device))))
        return;

    device->GetImmediateContext(&devicecontext);

    void** OriginalVTable = *(void***)(swapchain);
    constexpr size_t VTableSize = 18;

    auto ShadowVTable = std::make_unique<void* []>(VTableSize);
    memcpy(ShadowVTable.get(), OriginalVTable, sizeof(void*) * VTableSize);

    originalpresent = (presentfn)(OriginalVTable[8]);
    ShadowVTable[8] = (void*)(&present);

    originalresizebuffers = (resizebuffersfn)(OriginalVTable[13]);
    ShadowVTable[13] = (void*)(&resizebuffers);

    *(void***)(swapchain) = ShadowVTable.release();
    originalwindowproc = (WNDPROC)(SetWindowLongPtrW(windowhandle, GWLP_WNDPROC, (LONG_PTR)(windowprochandler)));
}

LRESULT CALLBACK renderer::windowprochandler(HWND hwnd, std::uint32_t msg, std::uint64_t wparam, std::int64_t lparam) {
    if (msg == WM_KEYDOWN) {
        if (wparam == VK_INSERT || wparam == VK_DELETE || wparam == VK_END) {
            test = !test;
        }
    }
    else if (msg == WM_DPICHANGED) {
        dpi_scale = LOWORD(wparam) / 96.0f;
    }
    /*else if ( msg == WM_SIZE ) {
        std::uint32_t width = LOWORD( lparam ), height = HIWORD( lparam );
        if ( !is_render_hooked && ( window_width != width || window_height != height ) ) {
            window_width = width, window_height = height;
            window_size_changes.push( true );
        }
    }*/

    if (test && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
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
        if (test)
            return true;
        break;
    }

    return CallWindowProc(originalwindowproc, hwnd, msg, wparam, lparam);
}

HRESULT WINAPI renderer::resizebuffers(IDXGISwapChain* InSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (niggachain::Renderer) // IF CRASH OR SMTH THEN IT MIGHT BE RELATED TO TS
        saferelease(rendertargetview);

    return originalresizebuffers(InSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}


bool renderer::initializeimgui() {
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

HRESULT WINAPI renderer::present(IDXGISwapChain* InSwapChain, UINT SyncInterval, UINT Flags) {
    static std::once_flag InitFlag;
    std::call_once(InitFlag, []() { if (niggachain::Renderer) initializeimgui(); });

    if (!niggachain::Renderer) return S_OK;

    if (!rendertargetview) {
        ID3D11Texture2D* BackBuffer = nullptr;

        if (SUCCEEDED(InSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer)))) {
            device->CreateRenderTargetView(BackBuffer, nullptr, &rendertargetview);
            BackBuffer->Release();
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (test) {

        g_user_interface->render();

    }

    ImGui::Render();
    devicecontext->OMSetRenderTargets(1, &rendertargetview, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return originalpresent(InSwapChain, SyncInterval, Flags);
}