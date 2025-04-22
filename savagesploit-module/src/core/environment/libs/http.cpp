//
// Created by savage on 18.04.2025.
//

#include "lgc.h"
#include "../environment.h"
#include "cpr/api.h"
#include "cpr/response.h"
#include "httpstatus/httpstatus.h"
#include "nlohmann/json.h"

enum request_methods {
	H_GET,
	H_HEAD,
	H_POST,
	H_PUT,
	H_DELETE,
	H_OPTIONS
};

std::map<std::string, request_methods> methods = {
	{ "get", H_GET },
	{ "head", H_HEAD },
	{ "post", H_POST },
	{ "put", H_PUT },
	{ "delete", H_DELETE },
	{ "options", H_OPTIONS }
};


int request(lua_State* L) {
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_getfield(L, 1, "Url");
	if (lua_type(L, -1) != LUA_TSTRING)
		luaL_error(L, "the url field is either missing or invalid");

	std::string url = lua_tostring(L, -1);
	if (url.empty())
		luaL_error(L, "no url provided");

	lua_pop(L, 1);

	auto method = H_GET;

	lua_getfield(L, 1, "Method");
	if (lua_type(L, -1) == LUA_TSTRING) {
		std::string method_string = luaL_checkstring(L, -1);
		std::transform(method_string.begin(), method_string.end(), method_string.begin(), tolower);

		if (!methods.count(method_string))
			luaL_error(L, "the request type '%s' is not valid", method_string.c_str());

		method = methods[method_string];
	}

	lua_pop(L, 1);

    auto headers = std::map<std::string, std::string, cpr::CaseInsensitiveCompare>();
    //Headers["Exploit-Guid"] = gw;
    //Headers["Nova-Fingerprint"] = gw;

	bool user_agent = false;

	lua_getfield(L, 1, "Headers");

	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING) {
				std::string key = lua_tostring(L, -2);
				std::string value = lua_tostring(L, -1);

				if (key == "User-Agent")
					user_agent = true;

                headers[key] = value;
			}
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);

	if (!user_agent)
        headers["User-Agent"] = "Nova/1.0";

	lua_getglobal(L, "game");
	lua_getfield(L, -1, "JobId");
	std::string job_id = lua_tostring(L, -1);
	lua_pop(L, 2);

	using Json = nlohmann::json;
	Json SessionId;
	SessionId["GameId"] = job_id.c_str();
	SessionId["PlaceId"] = job_id.c_str();

    headers["Roblox-Session-Id"] = SessionId.dump();
    headers["Roblox-Place-Id"] = job_id;
    headers["Roblox-Game-Id"] = job_id;
    headers["ExploitIdentifier"] = "Nova";

    cpr::Cookies cookies;
	lua_getfield(L, 1, "Cookies");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
            if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
                luaL_argerrorL(L, 1, "expected valid cookie key and value");
            }

            cookies.emplace_back(cpr::Cookie{_strdup(lua_tostring(L, -2)), _strdup(lua_tostring(L, -1))});

            lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	std::string body = "";
	lua_getfield(L, 1, "Body");
	if (lua_type(L, -1) == LUA_TSTRING)
		body = lua_tostring(L, -1);

	lua_pop(L, 1);

    cpr::Session request_session;
    request_session.SetUrl(url);
    request_session.SetHeader(cpr::Header{headers});
    request_session.SetCookies(cookies);
    request_session.SetBody(body);

    cpr::Response response;

    switch (method) {
        case H_GET:
            response = request_session.Get();
            break;
        case H_POST:
            response = request_session.Post();
            break;
        case H_PUT:
            response = request_session.Put();
            break;
        case H_DELETE:
            response = request_session.Delete();
            break;
        default:
            response.status_code = -999;
            break;
    }

    if (response.status_code == -999) {
        luaL_error(L, "the request method is invalid");
    }

	lua_rawcheckstack(L, 1);
	luaC_threadbarrier(L);

    lua_newtable(L);

    lua_pushboolean(L, !HttpStatus::IsError(response.status_code) &&
                       response.status_code != 0);
    lua_setfield(L, -2, "Success");

    lua_pushinteger(L, response.status_code);
    lua_setfield(L, -2, "StatusCode");

    lua_pushstring(L, response.text.c_str());
    lua_setfield(L, -2, "Body");

    lua_pushstring(L, response.status_line.c_str());
    lua_setfield(L, -2, "StatusMessage");

    lua_newtable(L);
    for (const auto &header: response.header) {
        lua_pushstring(L, header.second.c_str());
        lua_setfield(L, -2, header.first.c_str());
    }
    lua_setfield(L, -2, "Headers");

    lua_newtable(L);
    for (const auto &cookie: response.cookies) {
        lua_pushstring(L, cookie.GetValue().c_str());
        lua_setfield(L, -2, cookie.GetName().c_str());
    }
    lua_setfield(L, -2, "Cookies");

    return 1;
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
