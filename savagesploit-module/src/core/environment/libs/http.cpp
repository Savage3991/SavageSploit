//
// Created by savage on 18.04.2025.
//

#include "../environment.h"

int request(lua_State* L) {
    return 0;
}

int environment::http_get(lua_State *L) {
    return 0;
}


void environment::load_http_lib(lua_State *L) {
    static const luaL_Reg http[] = {
        {"request", request},
        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = http; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
