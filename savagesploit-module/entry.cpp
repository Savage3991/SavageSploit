#include "entry.h"

#include "globals.h"
#include "lstate.h"
#include "lualib.h"
#include "Windows.h"
#include "thread"
#include "src/core/environment/environment.h"
#include "src/core/execution/execution.h"
#include "src/core/render/render.h"

#include "src/rbx/engine/game.h"
#include "src/rbx/engine/hyperion.h"
#include "src/rbx/taskscheduler/taskscheduler.h"

// CXX EXCEPTION SUPPORT
namespace exceptions {
    std::uintptr_t image_base = 0;
    int image_size = 0;

    #define BASE_ALIGNMENT		0x10

    #define EH_MAGIC_NUMBER1        0x19930520
    #define EH_PURE_MAGIC_NUMBER1   0x01994000
    #define EH_EXCEPTION_NUMBER     ('msc' | 0xE0000000)

    #define VEHDATASIG_64 0xB16B00B500B16A33
    #define VEHDATASIG VEHDATASIG_64

    #pragma optimize( "", off )
    LONG CALLBACK VectoredHandlerShell(EXCEPTION_POINTERS * ExceptionInfo)
    {
        if (ExceptionInfo->ExceptionRecord->ExceptionCode == EH_EXCEPTION_NUMBER)
        {
            if (ExceptionInfo->ExceptionRecord->ExceptionInformation[2] >= image_base && ExceptionInfo->ExceptionRecord->ExceptionInformation[2] < image_base + image_size)
            {
                if (ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == EH_PURE_MAGIC_NUMBER1 && ExceptionInfo->ExceptionRecord->ExceptionInformation[3] == 0)
                {
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[0] = (ULONG_PTR)EH_MAGIC_NUMBER1;

                    ExceptionInfo->ExceptionRecord->ExceptionInformation[3] = image_base;
                }
            }
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }
}

void entry_point::entry(HMODULE DllModule) {
    rbx::standard_out::printf(rbx::message_type::message_output, "hello world");

#pragma region cxx exception
    exceptions::image_base = reinterpret_cast<std::uintptr_t>(DllModule);
    auto* Dos = reinterpret_cast<IMAGE_DOS_HEADER*>(exceptions::image_base);
    auto* Nt = reinterpret_cast<IMAGE_NT_HEADERS*>(exceptions::image_base + Dos->e_lfanew);
    auto* Opt = &Nt->OptionalHeader;
    exceptions::image_size = Opt->SizeOfImage;
    AddVectoredExceptionHandler(1, exceptions::VectoredHandlerShell);
#pragma endregion

    uintptr_t script_context = g_taskscheduler->get_script_context();
    globals::script_context = script_context;
    rbx::standard_out::printf(rbx::message_type::message_info, "script_context: %p", script_context);

    uintptr_t datamodel = *reinterpret_cast<uintptr_t*>(script_context + rbx::offsets::instance::parent);
    globals::datamodel = datamodel;
    rbx::standard_out::printf(rbx::message_type::message_info, "datamodel: %p", datamodel);

    lua_State* roblox_state = g_taskscheduler->get_roblox_state();
    globals::roblox_state = roblox_state;
    rbx::standard_out::printf(rbx::message_type::message_info, "roblox_state: %p", roblox_state);

    lua_State* our_state = lua_newthread(roblox_state);
    globals::our_state = our_state;

    lua_ref(roblox_state, -1);
    luaL_sandboxthread(our_state);

    our_state->userdata->identity = 8;
    our_state->userdata->capabilities = g_execution->capabilities;

    rbx::standard_out::printf(rbx::message_type::message_info, "our_state: %p", our_state);

//    g_execution->run_code(our_state, "print('helloooooo')");

    rbx::hyperion::add_to_cfg((void*)g_taskscheduler->initialize_hook);
    rbx::hyperion::add_to_cfg((void*)g_render->initialize);

    g_taskscheduler->initialize_hook();

    g_environment->initialize(our_state);

    g_taskscheduler->queue_script("print('hello ma niggers')");

    //g_render->initialize();

    g_taskscheduler->queue_script("printidentity()");
    g_taskscheduler->queue_script("print(identifyexecutor())");
    g_taskscheduler->queue_script("test()");







    // massive cock.
   // g_taskscheduler->set_fps(99999);

    //rbx::standard_out::printf(rbx::message_type::message_info, "luagc: %d", g_taskscheduler->get_fps());

    // base+5AD2768 = false

  //  *reinterpret_cast<bool*>(rbx::rvas::taskscheduler::fps_limit) = false;







    rbx::standard_out::printf(rbx::message_type::message_output, "done");

    if (!globals::initialized) {
        globals::initialized = true;
        while (true) {} // i don't want our thread to shit itself and go bai bai :P
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            entry_point e_p;
            std::thread(&entry_point::entry, &e_p, hModule).detach();
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
