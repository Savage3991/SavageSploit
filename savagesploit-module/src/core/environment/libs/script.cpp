//
// Created by savage on 22.04.2025.
//

#include "lstate.h"
#include "../environment.h"
#include "src/rbx/engine/game.h"

int getcallingscript(lua_State* L) {

    if (L->userdata->script.expired())
        lua_pushnil(L);
    else
        rbx::lua_bridge::push_1(L, L->userdata->script);

    return 1;
}

void environment::load_script_lib(lua_State *L) {
    static const luaL_Reg script[] = {
        {"getcallingscript", getcallingscript},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = script; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
