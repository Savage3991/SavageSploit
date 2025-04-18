//
// Created by savage on 17.04.2025.
//

#include "taskscheduler.h"

#include "globals.h"
#include "../engine/game.h"
#include "src/core/execution/execution.h"

uintptr_t taskscheduler::get_job_by_name(std::string job_name) {
    uintptr_t base = (uintptr_t)GetModuleHandleA("RobloxPlayerBeta.exe");
    uintptr_t taskscheduler = base + rbx::rvas::taskscheduler::taskschedulermk2;

    uintptr_t jobret = 0;
    uintptr_t jobstart = *(uintptr_t*)(taskscheduler + rbx::offsets::taskscheduler::job_start);
    uintptr_t jobend = *(uintptr_t*)(taskscheduler + rbx::offsets::taskscheduler::job_end);

    while (jobstart != jobend) {
        if (*(std::string*)(*(uintptr_t*)(jobstart)+rbx::offsets::taskscheduler::job_name) == job_name) {
            jobret = *(uintptr_t*)jobstart;
        }
        jobstart += 0x10;
    }
    return jobret;
}

uintptr_t taskscheduler::get_script_context() {
    uintptr_t base = (uintptr_t)GetModuleHandleA("RobloxPlayerBeta.exe");
    uintptr_t taskscheduler = base + rbx::rvas::taskscheduler::taskschedulermk2;

    uintptr_t luagc = 0;
    uintptr_t jobstart = *(uintptr_t*)(taskscheduler + rbx::offsets::taskscheduler::job_start);
    uintptr_t jobend = *(uintptr_t*)(taskscheduler + rbx::offsets::taskscheduler::job_end);

    while (jobstart != jobend) {
        if (*(std::string*)(*(uintptr_t*)(jobstart)+rbx::offsets::taskscheduler::job_name) == "LuaGc") {

            // We do this so we wont get the Homepage LuaGc :P
            uintptr_t job = *(uintptr_t*)jobstart;
            uintptr_t script_context_ptr = *(uintptr_t*)(job + rbx::offsets::luagc::script_context);
            uintptr_t datamodel = *(uintptr_t*)(script_context_ptr + rbx::offsets::instance::parent);
            uintptr_t datamodel_name = *(uintptr_t*)(datamodel + rbx::offsets::instance::name);
            std::string datamodel_name_t = *(std::string*)(datamodel_name);

            if (datamodel_name_t == "Ugc") {
                luagc = *(uintptr_t*)jobstart;
                break;
            }
        }
        jobstart += 0x10;
    }

    if (!luagc)
        MessageBoxA(NULL, "Failed to initialize (1)", "SavageSploit", MB_OK);

    uintptr_t script_context_ptr = *(uintptr_t*)(luagc + rbx::offsets::luagc::script_context);
    if (!script_context_ptr)
        MessageBoxA(NULL, "Failed to initialize (2)", "SavageSploit", MB_OK);

    return script_context_ptr;
}

lua_State* taskscheduler::get_roblox_state() {
    uintptr_t global_state = get_script_context() + rbx::offsets::script_context::global_state;

    uintptr_t identity = 0;
    uintptr_t base_instance = 0;

    return rbx::script_context::decrypt_state(rbx::script_context::get_global_state(global_state, &identity, &base_instance) + rbx::offsets::script_context::decrypt_state);
}

void taskscheduler::set_fps(double fps) {
    uintptr_t base = (uintptr_t)GetModuleHandleA("RobloxPlayerBeta.exe");
    uintptr_t taskscheduler = base + rbx::rvas::taskscheduler::taskschedulermk2;

    static const double min_frame_delay = 1.0 / 10000.0;
    double frame_delay = fps <= 0.0 ? min_frame_delay : 1.0 / fps;

    *reinterpret_cast<double*>(taskscheduler + rbx::offsets::taskscheduler::max_fps) = frame_delay;
}

int taskscheduler::get_fps() {
    uintptr_t base = (uintptr_t)GetModuleHandleA("RobloxPlayerBeta.exe");
    uintptr_t taskscheduler = base + rbx::rvas::taskscheduler::taskschedulermk2;

    return 1 / *reinterpret_cast<double*>(taskscheduler + rbx::offsets::taskscheduler::max_fps);
}

int taskscheduler::scheduler_hook() {
    if (!queue.empty()) {
        std::string top = queue[0];
        if (top.empty())
            return 0;

        queue.erase(queue.begin());

        g_execution->run_code(globals::our_state, top);
    }

    return 0;
}

int scheduler_hook_1(lua_State* L) {
    g_taskscheduler->scheduler_hook();

    return 0;
}

void taskscheduler::initialize_hook() {
    lua_State* L = globals::our_state;
    lua_getglobal(L, "game");
    lua_getfield(L, -1, "GetService");
    lua_pushvalue(L, -2);
    lua_pushstring(L, "RunService");
    lua_pcall(L, 2, 1, 0);

    lua_getfield(L, -1, "Stepped");
    lua_getfield(L, -1, "Connect");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, scheduler_hook_1, nullptr, 0);
    lua_pcall(L, 2, 0, 0);
    lua_pop(L, 3);
}

void taskscheduler::queue_script(std::string script) {
    queue.push_back(script);
}


