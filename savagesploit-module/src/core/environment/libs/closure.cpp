//
// Created by savage on 18.04.2025.
//

#include "../environment.h"

int loadstring(lua_State* L) {
    return 0;
}

void environment::load_closure_lib(lua_State *L) {
    static const luaL_Reg closure[] = {
        {"loadstring", loadstring},
        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = closure; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
