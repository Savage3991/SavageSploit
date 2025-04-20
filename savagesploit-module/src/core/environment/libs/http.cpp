//
// Created by savage on 18.04.2025.
//

#include "../environment.h"
#include "cpr/api.h"
#include "cpr/response.h"

int request(lua_State* L) {
    return 0;
}

int environment::http_get(lua_State *L) {
    std::string url;

    if (lua_isstring(L, 2)) {
        url = lua_tostring(L, 2);
    }
    else if (lua_isstring(L, 1)) {
        url = lua_tostring(L, 1);
    }
    else {
        luaL_typeerror(L, 1, "string");
    }

    cpr::Response response;
    response = cpr::Get(cpr::Url{url}, cpr::Header{{"accept", "application/json"}, {"User-Agent", "Roblox/WinIet"}});

    lua_pushlstring(L, response.text.data(), response.text.size());


    return 1;
}

void environment::load_http_lib(lua_State *L) {
    static const luaL_Reg http[] = {
        {"request", request},
        {"httpget", http_get},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = http; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
