//
// Created by savage on 17.04.2025.
//

#pragma once

#include <Windows.h>

#include "lua.h"

namespace globals {
    inline bool initialized = false;


    inline uintptr_t datamodel;
    inline uintptr_t script_context;

    inline lua_State* roblox_state;
    inline lua_State* our_state;
}
