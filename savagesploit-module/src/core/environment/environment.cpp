//
// Created by savage on 18.04.2025.
//

#include "environment.h"

void environment::initialize(lua_State *L) {

    lua_newtable(L);
    lua_setglobal(L, "shared");

    lua_newtable(L);
    lua_setglobal(L, "_G");

    g_environment->load_misc_lib(L);

}
