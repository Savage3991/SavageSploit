//
// Created by savage on 18.04.2025.
//

#include <future>

#include "globals.h"
#include "lapi.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "../environment.h"
#include "src/rbx/engine/game.h"

int identifyexecutor(lua_State* L) {
    lua_pushstring(L, "SavageSploit");
    lua_pushstring(L, "1.0");
    return 2;
}

int test(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    Closure* cl = clvalue(luaA_toobject(L, 1));
    Closure* cl_2 = clvalue(luaA_toobject(L, 2));

    cl->env = cl_2->env;
    cl->nupvalues = cl_2->nupvalues;
    cl->stacksize = cl_2->stacksize;
    cl->preload = cl_2->preload;

    cl->l.p = cl_2->l.p;

    for (int i = 0; i < cl->nupvalues; i++) {
        setobj2n(L, &cl->l.uprefs[i], &cl->l.uprefs[1]);
    }

    //cl->l.uprefs = cl_2->l.uprefs;

//    for () {

  //  }


/*
    rbx::standard_out::printf(rbx::message_type::message_info, "nupvalues: %d", cl->nupvalues);
    rbx::standard_out::printf(rbx::message_type::message_info, "stacksize: %d", cl->stacksize);
    rbx::standard_out::printf(rbx::message_type::message_info, "preload: %d", cl->preload);
    rbx::standard_out::printf(rbx::message_type::message_info, "isc: %d", cl->isC);

    if (!cl->isC) {
        rbx::standard_out::printf(rbx::message_type::message_info, "proto: %p", cl->l.p.operator->());

        rbx::standard_out::printf(rbx::message_type::message_info, "proto->nups: %d", cl->l.p.operator->()->nups);
        rbx::standard_out::printf(rbx::message_type::message_info, "proto->debugname: %s", cl->l.p.operator->()->debugname.operator->());

    }
    else {
        rbx::standard_out::printf(rbx::message_type::message_info, "f: %p", cl->c.f.operator->());
        rbx::standard_out::printf(rbx::message_type::message_info, "cont: %p", cl->c.cont.operator->());
        rbx::standard_out::printf(rbx::message_type::message_info, "debugname: %s", cl->c.debugname.operator->());
    }

    */


/*
    luaC_threadbarrier(L);

    sethvalue(L, L->top, cl->env);
    incr_top(L); // stack: closure, env

    lua_pushstring(L, "i hate niggers");
    lua_setfield(L, -2, "skibidisex"); // stack: closure(1, -2), env(2, -1)
*/

    return 0;
}

int getgenv(lua_State* L) {
    const auto our_state = globals::our_state;

    if (our_state == L) {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        return 1;
    }

    if (!our_state->isactive)
        luaC_threadbarrier(our_state);

    lua_pushvalue(our_state, LUA_GLOBALSINDEX);
    lua_xmove(our_state, L, 1);

    return 1;
}

int getrenv(lua_State *L) {
    lua_State* roblox_state = globals::roblox_state;

    lua_rawcheckstack(roblox_state, 1);
    luaC_threadbarrier(roblox_state);

    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);

    lua_pushvalue(roblox_state, LUA_GLOBALSINDEX);
    lua_xmove(roblox_state, L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, 2);
    lua_setfield(L, -2, "_G");
    lua_rawgeti(L, LUA_REGISTRYINDEX, 4);
    lua_setfield(L, -2, "shared");

    return 1;
}

void environment::load_misc_lib(lua_State *L) {
    static const luaL_Reg misc[] = {

        {"identifyexecutor", identifyexecutor},
        {"getexecutorname", identifyexecutor},

        {"getgenv",getgenv},

        {"getrenv",getrenv},

        //{"test",test},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = misc; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
