//
// Created by savage on 20.04.2025.
//

#include "../environment.h"

int getrawmetatable(lua_State* L) {
    luaL_checkany(L, 1);

    bool has_metatable = lua_getmetatable(L, 1);

    if (!has_metatable)
        lua_pushnil(L);

    return 1;
}

void environment::load_metatable_lib(lua_State *L) {
    static const luaL_Reg metatable[] = {
        {"getrawmetatable", getrawmetatable},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = metatable; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
