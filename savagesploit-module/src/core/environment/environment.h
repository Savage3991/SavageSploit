//
// Created by savage on 18.04.2025.
//

#pragma once

#include <lua.h>
#include <lualib.h>

class environment {

public:
    static void initialize(lua_State* L);


    // LIBS
    void load_http_lib(lua_State* L);
    void load_closure_lib(lua_State* L);
    void load_misc_lib(lua_State* L);


    // FUNCS FOR GAME HOOK
    int http_get(lua_State* L);
    int get_objects(lua_State* L);
};

inline const auto g_environment = std::make_unique<environment>();