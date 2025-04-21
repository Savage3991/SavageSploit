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

int hookmetamethod(lua_State* L) {
    if (!lua_isuserdata(L, 1) && !lua_istable(L, 1)) {
        luaL_error(L, "Expected userdata or table as first argument.");
    }
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    const char* metamethod = lua_tostring(L, 2);

    if (!lua_getmetatable(L, 1)) {
        luaL_error(L, "No metatable found");
    }

    lua_pushstring(L, metamethod);
    lua_rawget(L, -2);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Unable to find metamethod");
    }

    lua_getglobal(L, "hookfunction");
    if (!lua_isfunction(L, -1)) {
        luaL_error(L, "Unable to get hookmetamethod");
    }

    lua_pushvalue(L, -2);
    lua_pushvalue(L, 3);

    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}
void environment::load_metatable_lib(lua_State *L) {
    static const luaL_Reg metatable[] = {
        {"getrawmetatable", getrawmetatable},
        {"hookmetamethod", hookmetamethod},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = metatable; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
