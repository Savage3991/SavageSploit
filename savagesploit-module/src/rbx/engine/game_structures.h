//
// Created by savage on 17.04.2025.
//

#pragma once

#include <Windows.h>
struct lua_State;
struct Proto;
struct global_State;
struct LuaTable;

namespace rbx {

    enum message_type {
        message_output = 0,
        message_info = 1,
        message_warning = 2,
        message_error = 3
    };

}
