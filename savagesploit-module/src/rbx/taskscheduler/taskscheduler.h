//
// Created by savage on 17.04.2025.
//

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "lua.h"

class taskscheduler {
    std::vector<std::string> queue;
public:
    // BASIC TASKSCHEDULER STUFF
    uintptr_t get_job_by_name(std::string);

    // MORE "ADVANCED" STUFF
    uintptr_t get_script_context();
    lua_State* get_roblox_state();

    // FPS SHI
    void set_fps(double fps);
    int get_fps();

    // TASKSCHEDULER HOOK
    int scheduler_hook();

    static void initialize_hook();
    void queue_script(std::string);
};

inline const auto g_taskscheduler = std::make_unique<taskscheduler>();