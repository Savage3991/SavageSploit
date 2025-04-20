//
// Created by savage on 17.04.2025.
//

#pragma once
#include <string>
#include "lua.h"
#include "lapi.h"
#include "lstate.h"
#include "lualib.h"

class execution {
public:
    uintptr_t capabilities = 0xFFFFFFFFFFFFFFFF;

    void set_capabilities(Proto *proto, uintptr_t* caps);

    std::string compile(std::string);

    int load_string(lua_State*, std::string, std::string);

    bool run_code(lua_State*, std::string);
};

inline const auto g_execution = std::make_unique<execution>();