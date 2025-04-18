//
// Created by savage on 18.04.2025.
//

#include "../environment.h"

int identifyexecutor(lua_State* L) {
    lua_pushstring(L, "SavageSploit");
    lua_pushstring(L, "1.0");
    return 2;
}

int test(lua_State* L) {
    luaL_error(L, "hi");
    return 0;
}

void environment::load_misc_lib(lua_State *L) {
    static const luaL_Reg misc[] = {
        {"identifyexecutor", identifyexecutor},
        {"getexecutorname", identifyexecutor},
        {"test", test},
        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = misc; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
