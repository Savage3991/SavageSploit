//
// Created by savage on 21.04.2025.
//

#include "../environment.h"
#include <filesystem>
#include <fstream>

#include "src/core/execution/execution.h"

void check_workspace_folder() {
    if (!std::filesystem::exists("workspace"))
        std::filesystem::create_directory("workspace");
}

int appendfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);

    check_workspace_folder();

    std::string filename = "workspace/" + std::string(lua_tostring(L, 1));

    if (!std::filesystem::exists(filename))
        luaL_error(L, "file doesnt exist");

    std::ifstream in_file(filename);
    if (!in_file.is_open())
        luaL_error(L, "couldnt read file");

    std::stringstream buffer;
    buffer << in_file.rdbuf();
    std::string existing_content = buffer.str();
    in_file.close();

    std::ofstream out_file(filename);
    if (out_file.is_open()) {
        out_file << existing_content << lua_tostring(L, 2);
        out_file.close();
    }
    else {
        luaL_error(L, "couldnt access file");
    }

    return 0;
}

int writefile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);

    check_workspace_folder();

    std::string filename = "workspace/" + std::string(lua_tostring(L, 1));
    std::ofstream file(filename);

    if (file.is_open()) {
        file << lua_tostring(L, 2);
        file.close();
    }
    else {
        luaL_error(L, "couldnt access file");
    }

    return 0;
}

int readfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    if (!std::filesystem::exists("workspace/" + std::string(lua_tostring(L, 1))))
        luaL_error(L, "file doesnt exist");

    std::ifstream file("workspace/" + std::string(lua_tostring(L, 1)));

    if (!file.is_open())
        luaL_error(L, "cant read file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    lua_pushlstring(L, content.data(), content.size());
    return 1;
}

int delfolder(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    std::string filename = "workspace/" + std::string(lua_tostring(L, 1));

    if (!std::filesystem::exists(filename))
        luaL_error(L,"directory doesnt exist");

    std::filesystem::remove_all(filename);

    return 0;
}

int delfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    std::string filename = "workspace/" + std::string(lua_tostring(L, 1));

    if (!std::filesystem::exists(filename))
        luaL_error(L,"file doesnt exist");

    std::filesystem::remove(filename);

    return 0;
}

int isfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    lua_pushboolean(L, std::filesystem::is_regular_file("workspace/" + std::string(lua_tostring(L, 1))));

    return 1;
}

int isfolder(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    lua_pushboolean(L, std::filesystem::is_directory("workspace/" + std::string(lua_tostring(L, 1))));

    return 1;
}

int makefolder(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    std::filesystem::create_directory("workspace/" + std::string(lua_tostring(L, 1)));

    return 0;
}

int listfiles(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    std::filesystem::path workspace_path = "workspace";
    const auto path = "workspace/" + std::string(lua_tostring(L, 1));

    if (!std::filesystem::exists(path))
        luaL_argerrorL(L, 1, "path doesnt exist");

    lua_newtable(L);

    for (int i = 0; const auto& file_entry: std::filesystem::directory_iterator(path)) {
        std::string file_path = file_entry.path().string().substr(workspace_path.string().size() + 1);
        lua_pushstring(L, file_path.c_str());
        lua_rawseti(L, -2, ++i);
    }

    return 1;
}

int loadfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    check_workspace_folder();

    std::string filename = "workspace/" + std::string(lua_tostring(L, 1));

    if (!std::filesystem::exists(filename))
        luaL_error(L, "file doesnt exist");

    std::ifstream in_file(filename);
    if (!in_file.is_open())
        luaL_error(L, "couldnt read file");

    std::stringstream buffer;
    buffer << in_file.rdbuf();
    std::string existing_content = buffer.str();
    in_file.close();

    return g_execution->load_string(L, "", existing_content);
}

void environment::load_filesystem_lib(lua_State *L) {
    static const luaL_Reg file_system[] = {
        {"writefile", writefile},
        {"appendfile", appendfile},
        {"readfile",readfile},
        {"delfile", delfile},
        {"delfolder", delfolder},
        {"listfiles", listfiles},
        {"isfile", isfile},
        {"isfolder", isfolder},
        {"makefolder", makefolder},
        {"loadfile",loadfile},
        {"dofile", loadfile},

        {nullptr, nullptr}
    };

    for (const luaL_Reg* lib = file_system; lib->name; lib++) {
        lua_pushcclosure(L, lib->func, nullptr, 0);
        lua_setglobal(L, lib->name);
    }
}
