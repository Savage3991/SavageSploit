//
// Created by savage on 18.04.2025.
//

#include "environment.h"

#include "lstate.h"


lua_CFunction old_index;
lua_CFunction old_namecall;

int index_hook(lua_State* L) {
    const auto script_pointer = L->userdata->script;

    if (lua_isstring(L, 2) && script_pointer.expired()) {
        const char* data = luaL_checkstring(L, 2);

        if (!strcmp(data, "HttpGet") || !strcmp(data, "HttpGetAsync")) {
            lua_getglobal(L, "httpget");
            return 1;
        }
    }

    return old_index(L);
}
int namecall_hook(lua_State* L) {
    const auto script_pointer = L->userdata->script;

    if (L->namecall && script_pointer.expired()) {
        const char* data = L->namecall->data;

        if (!strcmp(data, "HttpGet") || !strcmp(data, "HttpGetAsync")) {
            return g_environment->http_get(L);
        }

    }

    return old_namecall(L);
}
void hook_index(lua_State *L) {
    lua_getglobal(L, "game"); // stack: ttable
    luaL_getmetafield(L, -1, "__index"); // stack: ttable, function

    auto* index = (Closure*)lua_topointer(L, -1);
    if (!index)
        MessageBoxA(NULL, "Failed to hook __index", "SavageSploit", MB_OK);

    lua_pop(L, 2); // stack: empty

    lua_CFunction index_func = index->c.f;
    old_index = index_func;
    index->c.f = index_hook;
}
void hook_namecall(lua_State *L) {
    lua_getglobal(L, "game"); // stack: ttable
    luaL_getmetafield(L, -1, "__namecall"); // stack: ttable, function

    auto* namecall = (Closure*)lua_topointer(L, -1);
    if (!namecall)
        MessageBoxA(NULL, "Failed to hook __namecall", "SavageSploit", MB_OK);

    lua_pop(L, 2); // stack: empty

    lua_CFunction namecall_func = namecall->c.f;
    old_namecall = namecall_func;
    namecall->c.f = namecall_hook;
}

void environment::initialize(lua_State *L) {

    lua_newtable(L);
    lua_setglobal(L, "shared");

    lua_newtable(L);
    lua_setglobal(L, "_G");

    g_environment->load_misc_lib(L);
    g_environment->load_http_lib(L);
    g_environment->load_closure_lib(L);
    g_environment->load_metatable_lib(L);
    g_environment->load_filesystem_lib(L);
    g_environment->load_script_lib(L);
    g_environment->load_crypt_lib(L);




    hook_index(L);
    hook_namecall(L);

}
