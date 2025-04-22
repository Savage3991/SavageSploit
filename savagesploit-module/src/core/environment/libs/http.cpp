//
// Created by savage on 18.04.2025.
//

#include "../environment.h"
#include "cpr/api.h"
#include "cpr/response.h"
#include "httpstatus/httpstatus.h"
#include "nlohmann/json.h"

enum RequestMethods {
	H_GET,
	H_HEAD,
	H_POST,
	H_PUT,
	H_DELETE,
	H_OPTIONS
};

std::map<std::string, RequestMethods> Methods = {
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
		luaL_error(L, "The 'Url' field in the request table is either invalid or missing.");

	std::string Url = lua_tostring(L, -1);

	if (Url.empty())
		luaL_error(L, "no url provided to request");

	if (Url.find("http://") != 0 && Url.find("https://") != 0)
		luaL_error(L, "The specified protocol is invalid (expected 'http://' or 'https://')");

	lua_pop(L, 1);

	auto Method = H_GET;

	lua_getfield(L, 1, "Method");
	if (lua_type(L, -1) == LUA_TSTRING) {
		std::string MethodString = luaL_checkstring(L, -1);
		std::transform(MethodString.begin(), MethodString.end(), MethodString.begin(), tolower);

		if (!Methods.count(MethodString))
			luaL_error(L, "The request type '%s' is not a valid HTTP request method.", MethodString.c_str());

		Method = Methods[MethodString];
	}

	lua_pop(L, 1);

    auto Headers = std::map<std::string, std::string, cpr::CaseInsensitiveCompare>();
	//Headers("Exploit-Guid", HardwareIdentifier);
	//Headers("Nova-Fingerprint", HardwareIdentifier);
    //Headers["Exploit-Guid"] = HardwareIdentifier;
    //Headers["Nova-Fingerprint"] = HardwareIdentifier;

	bool HasUserAgent = false;

	lua_getfield(L, 1, "Headers");

	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING) {
				std::string Key = lua_tostring(L, -2);
				std::string Value = lua_tostring(L, -1);

				if (Key == "User-Agent")
					HasUserAgent = true;

                Headers[Key] = Value;
				//Headers.Insert(Key, Value);
			}
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);

	if (!HasUserAgent)
        Headers["User-Agent"] = "Nova/1.0";

	lua_getglobal(L, "game");
	lua_getfield(L, -1, "JobId");
	std::string jobId = lua_tostring(L, -1);
	lua_pop(L, 2);

	using Json = nlohmann::json;
	Json SessionId;
	SessionId["GameId"] = jobId.c_str();
	SessionId["PlaceId"] = jobId.c_str();

    Headers["Roblox-Session-Id"] = SessionId.dump();
    Headers["Roblox-Place-Id"] = jobId;
    Headers["Roblox-Game-Id"] = jobId;
    Headers["ExploitIdentifier"] = "Nova";

    cpr::Cookies Cookies;
	lua_getfield(L, 1, "Cookies");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
            if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
                luaL_argerrorL(L, 1, "Expected Valid Cookie Key and Value");
            }

            Cookies.emplace_back(cpr::Cookie{_strdup(lua_tostring(L, -2)), _strdup(lua_tostring(L, -1))});

            lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	std::string Body = "";
	lua_getfield(L, 1, "Body");
	if (lua_type(L, -1) == LUA_TSTRING)
		Body = lua_tostring(L, -1);

	lua_pop(L, 1);

    cpr::Session RequestSession;
    RequestSession.SetUrl(Url);
    RequestSession.SetHeader(cpr::Header{Headers});
    RequestSession.SetCookies(Cookies);
    RequestSession.SetBody(Body);

    cpr::Response Response;

    switch (Method) {
        case H_GET:
            Response = RequestSession.Get();
            break;
        case H_POST:
            Response = RequestSession.Post();
            break;
        case H_PUT:
            Response = RequestSession.Put();
            break;
        case H_DELETE:
            Response = RequestSession.Delete();
            break;
        default:
            Response.status_code = -999;
            break;
    }

    if (Response.status_code == -999) {
        luaL_error(L, "The specified request method is invalid.");
    }

    lua_newtable(L);

    lua_pushboolean(L, !HttpStatus::IsError(Response.status_code) &&
                       Response.status_code != 0);
    lua_setfield(L, -2, "Success");

    lua_pushinteger(L, Response.status_code);
    lua_setfield(L, -2, "StatusCode");

    lua_pushstring(L, Response.text.c_str());
    lua_setfield(L, -2, "Body");

    lua_pushstring(L, Response.status_line.c_str());
    lua_setfield(L, -2, "StatusMessage");

    lua_newtable(L);
    for (const auto &Header: Response.header) {
        lua_pushstring(L, Header.second.c_str());
        lua_setfield(L, -2, Header.first.c_str());
    }
    lua_setfield(L, -2, "Headers");

    lua_newtable(L);
    for (const auto &Cookie: Response.cookies) {
        lua_pushstring(L, Cookie.GetValue().c_str());
        lua_setfield(L, -2, Cookie.GetName().c_str());
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
