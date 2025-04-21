//
// Created by savage on 18.04.2025.
//

#include <mutex>
#include <regex>
#include <unordered_map>

#include "lapi.h"
#include "lgc.h"
#include "lstate.h"
#include "../environment.h"
#include "src/core/execution/execution.h"

int loadstring(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING); // source

    std::string source = lua_tostring(L, 1);
    std::string chunkname = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";

    return g_execution->load_string(L, chunkname, source);
}

int checkcaller(lua_State* L) {
    if (L->userdata->script.expired())
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);

    return 1;
}

int clonefunction(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    Closure* cl = clvalue(luaA_toobject(L, 1));

    if (cl->isC) {
        // add here check for newcc later ig idk
        lua_clonecfunction(L, 1);
    }
    else {
        lua_clonefunction(L,1);
    }

    return 1;
}

int newlclosure(lua_State *L) {
    luaL_checktype(L, -1, LUA_TFUNCTION);

    lua_rawcheckstack(L, 3);
    luaC_threadbarrier(L);

    lua_newtable(L);
    lua_newtable(L); // Meta

    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);

    Closure *closure = clvalue(luaA_toobject(L, -3));

    L->top->value.p = closure->env;
    L->top->tt = ::lua_Type::LUA_TTABLE;
    L->top++;
    lua_setfield(L, -2, "__index");
    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);
    L->top->value.p = closure->env;
    L->top->tt = ::lua_Type::LUA_TTABLE;
    L->top++;
    lua_setfield(L, -2, "__newindex");
    lua_setreadonly(L, -1, true);
    lua_setmetatable(L, -2);

    lua_pushvalue(L, -2);
    lua_rawsetfield(L, -2, "new_l_closure_wrap");

    std::string bytecode = g_execution->compile("return new_l_closure_wrap(...)");
    luau_load(L, "@", bytecode.c_str(), bytecode.size(), -1);

    g_execution->set_capabilities(clvalue(luaA_toobject(L, -1))->l.p, &g_execution->capabilities);

    lua_remove(L, lua_gettop(L) - 1); // Balance lua stack.
    return 1;
}

namespace new_closure {
    template< class T >
    class safe_storage_t {
        std::mutex mutex;
    protected:
        static inline T container = { };
    public:
        auto safe_request( auto request, auto... args ) noexcept {
            std::unique_lock l{ mutex };
            return request( args... );
        };

        void clear() noexcept {
            safe_request( [ & ]( ) {
                container.clear();
            } );
        }
    };

    class newcclosure_cache_t : public safe_storage_t < std::unordered_map < Closure*, Closure* > > {
    public:
        void add( Closure* cclosure, Closure* lclosure ) noexcept {
            safe_request( [ & ]( ) {
                container[ cclosure ] = lclosure;
            } );
        };

        void remove( Closure* object ) noexcept {
            safe_request( [ & ] ( ) {
                auto it = container.find( object );
                if ( it != container.end( ) )
                    container.erase( it );
            } );
        };

        std::optional< Closure* > get( Closure* closure ) noexcept {
            return safe_request( [ & ]( ) -> std::optional< Closure* > {
                if ( container.contains( closure ) )
                    return container.at( closure );

                return std::nullopt;
            } );
        };
    } inline newcclosure_cache;

    std::string strip_error_message( const std::string& message ) {
        static auto callstack_regex = std::regex(  R"(.*"\]:(\d)*: )" , std::regex::optimize | std::regex::icase );
        if ( std::regex_search( message.begin( ), message.end( ), callstack_regex ) ) {
            const auto fixed = std::regex_replace( message, callstack_regex, "" );
            return fixed;
        }

        return message;
    };

    static void handler_run(lua_State* L, void* ud) {
        luaD_call( L, (StkId)( ud ), LUA_MULTRET );
    }

    std::int32_t handler_continuation( lua_State* L, std::int32_t status ) {
        if ( status != LUA_OK ) {
            const auto regexed_error = strip_error_message( lua_tostring( L, -1 ) );
            lua_pop( L, 1 );

            lua_pushlstring( L, regexed_error.c_str( ), regexed_error.size( ) );
            lua_error( L );
        }

        return lua_gettop( L );
    };

    std::int32_t handler( lua_State* L ) {
        const auto arg_count = lua_gettop( L );
        const auto closure = newcclosure_cache.get(clvalue(L->ci->func));//newcclosure_cache.find( clvalue( L->ci->func ) );

        if ( !closure )
            luaL_error( L,  "unable to find closure"  );

        setclvalue( L, L->top, *closure );
        L->top++;

        lua_insert( L, 1 );

        StkId func = L->base;
        L->ci->flags |= LUA_CALLINFO_HANDLE;

        L->baseCcalls++;
        int status = luaD_pcall( L, handler_run, func, savestack( L, func ), 0 );
        L->baseCcalls--;

        if ( status == LUA_ERRRUN ) {
            const auto regexed_error = strip_error_message( lua_tostring( L, -1 ) );
            lua_pop( L, 1 );

            lua_pushlstring( L, regexed_error.c_str( ), regexed_error.size( ) );
            lua_error( L );
            return 0;
        }

        expandstacklimit( L, L->top );

        if ( status == 0 && ( L->status == LUA_YIELD || L->status == LUA_BREAK ) )
            return -1;

        return lua_gettop( L );
    };

}

int newcclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if (lua_iscfunction(L,1)) {
        lua_pushnil(L);
    }
    else {
        lua_ref( L, 1 );
        lua_pushcclosurek( L, new_closure::handler, nullptr, 0, new_closure::handler_continuation );
        lua_ref( L, -1 );

        new_closure::newcclosure_cache.add(clvalue( luaA_toobject( L, -1 )), clvalue( luaA_toobject( L, 1 ) ));
    }

    return 1;
}

int hookfunction(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    Closure* original = clvalue(luaA_toobject(L,1));
    Closure* hook = clvalue(luaA_toobject(L, 2));
    lua_ref(L,1);
    lua_ref(L,2);

    // C CLOSURES
    if (original->isC && hook->isC) {
        // C->C

        lua_clonecfunction(L, 1);
        lua_ref(L,-1);

        for (int i = 0; i < hook->nupvalues; ++i)
            setobj2n(L, &original->c.upvals[i], &hook->c.upvals[i]);

        original->env = hook->env;
        original->stacksize = hook->stacksize;

        original->c.cont = hook->c.cont;
        original->c.f = hook->c.f;
    }
    else if (original->isC && !hook->isC) {
        // C->L

        lua_pushcclosure(L, newcclosure, nullptr, 0);
        lua_pushvalue(L, 2);
        lua_call(L, 1, 1);

        Closure* new_c_closure = clvalue(luaA_toobject(L, -1));
        lua_pop(L,1);

        lua_clonecfunction(L, 1);
        lua_ref(L,-1);

        for (int i = 0; i < new_c_closure->nupvalues; ++i)
            setobj2n(L, &original->c.upvals[i], &new_c_closure->c.upvals[i]);

        original->env = new_c_closure->env;
        original->stacksize = new_c_closure->stacksize;

        original->c.cont = new_c_closure->c.cont;
        original->c.f = new_c_closure->c.f;

        new_closure::newcclosure_cache.add(original, new_c_closure);
    }

    // L CLOSURE
    else if (!original->isC && !hook->isC) {
        // L->L

        lua_clonefunction(L, 1);
        lua_ref(L,-1);

        original->env = hook->env;
        original->nupvalues = hook->nupvalues;
        original->stacksize = hook->stacksize;
        original->preload = hook->preload;

        original->l.p = hook->l.p;

        for (int i = 0; i < hook->nupvalues; i++) {
            setobj2n(L, &original->l.uprefs[i], &hook->l.uprefs[i]);
        }
    }
    else if (!original->isC && hook->isC) {
        // L->C

        lua_rawcheckstack(L, 2);
        luaC_threadbarrier(L);

        lua_pushcclosure(L, newlclosure, nullptr, 0);
        lua_pushvalue(L, 2);
        lua_call(L, 1, 1);

        Closure* new_l_closure = clvalue(luaA_toobject(L, -1));
        lua_pop(L,1);

        original->env = new_l_closure->env;
        original->nupvalues = new_l_closure->nupvalues;
        original->stacksize = new_l_closure->stacksize;
        original->preload = new_l_closure->preload;

        original->l.p = new_l_closure->l.p;

        for (int i = 0; i < new_l_closure->nupvalues; i++) {
            setobj2n(L, &original->l.uprefs[i], &new_l_closure->l.uprefs[i]);
        }

        setclvalue(L, L->top, original);
        L->top++;
    }
    else {
        luaL_error(L, "unsupported hooking pair");
    }

    return 1;

}

int iscclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}

int islclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    lua_pushboolean(L, lua_isLfunction(L, 1));
    return 1;
}



void environment::load_closure_lib(lua_State *L) {
    static const luaL_Reg closure[] = {
        {"loadstring", loadstring},
        {"checkcaller", checkcaller},
        {"clonefunction", clonefunction},
        {"hookfunction", hookfunction},
        {"newcclosure", newcclosure},
        {"newlclosure", newlclosure},
        {"iscclosure", iscclosure},
        {"islclosure", islclosure},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = closure; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
