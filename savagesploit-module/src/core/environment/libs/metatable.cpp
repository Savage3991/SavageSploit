//
// Created by savage on 21/04/2025.
//
#include "globals.h"
#include "lapi.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "../environment.h"

int getrawmetatable(lua_State *L) {
    luaL_checkany(L, 1);

    if (!lua_getmetatable(L, 1))
        lua_pushnil(L);

    return 1;
}

int isreadonly(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    LuaTable *table = hvalue(luaA_toobject(L, 1));

    lua_pushboolean(L, static_cast<bool>(table->readonly));
    return 1;
}

int setrawmetatable(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_argexpected(L, lua_istable(L, 2) || lua_isuserdata(L,2) || lua_isnil(L, 2), 2, "table, userdata or nil");

    lua_setmetatable(L, 1);

    lua_pushvalue(L, 1);
    return 1;
}

int setreadonly(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    LuaTable *table = hvalue(luaA_toobject(L, 1));

    table->readonly = lua_toboolean(L, 2);
    return 0;
}

extern int hookfunction(lua_State *L);

int hookmetamethod(lua_State *L) {
    luaL_checkany(L, 1);
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    if (!luaL_getmetafield(L, 1, lua_tolstring(L, 2, nullptr)))
        luaL_argerrorL(L, 2, "metamethod does not exist");

    Closure *metamethod_closure = clvalue(luaA_toobject(L, -1));
    lua_pop(L, 1);

    lua_rawcheckstack(L, 3);
    luaC_threadbarrier(L);

    lua_pushcclosurek(L, hookfunction, nullptr, 0, nullptr);
    setclvalue(L, L->top, metamethod_closure);
    L->top++;
    lua_pushvalue(L, 3);
    lua_call(L, 2, 1);

    return 1;
}

int getnamecallmethod(lua_State* L) {
    if (auto namecall = L->namecall) {
        lua_pushstring(L, namecall->data);
        return 1;
    }
    return 0;
}

void environment::load_metatable_lib(lua_State *L) {
    static const luaL_Reg misc[] = {
            {"getrawmetatable", getrawmetatable},
            {"isreadonly", isreadonly},
            {"setrawmetatable", setrawmetatable},
            {"setreadonly", setreadonly},
            {"hookmetamethod", hookmetamethod},
            {"getnamecallmethod", getnamecallmethod},
            {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = misc; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}