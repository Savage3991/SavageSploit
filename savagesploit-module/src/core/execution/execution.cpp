//
// Created by savage on 17.04.2025.
//

#include "execution.h"

#include "Luau/Compiler.h"
#include "Luau/Bytecode.h"
#include "Luau/BytecodeBuilder.h"
#include "Luau/BytecodeUtils.h"

class bytecode_encoder_t : public Luau::BytecodeEncoder { // thx shade :P
    inline void encode(uint32_t* data, size_t count) override {
        for (auto i = 0; i < count;)
        {
            uint8_t op = LUAU_INSN_OP(data[i]);
            const auto oplen = Luau::getOpLength((LuauOpcode)op);
            BYTE* OpCodeLookUpTable = rbx::luau::opcode_table_lookup;
            uint8_t new_op = op * 227;
            new_op = OpCodeLookUpTable[new_op];
            data[i] = (new_op) | (data[i] & ~0xff);
            i += oplen;
        }
    }
};

void execution::set_capabilities(Proto *proto, uintptr_t* caps) {
    if (!proto)
        return;

    proto->userdata = caps;
    for (int i = 0; i < proto->sizep; ++i)
        set_capabilities(proto->p[i], caps);
}

std::string execution::compile(std::string code) {
    auto encoder = bytecode_encoder_t();

    const char *mutableglobals[] = {
        "Game", "Workspace", "game", "plugin", "script", "shared", "workspace",
        "_G", "_ENV", "_GENV", "_RENV",
        nullptr
    };

    auto compileoptions = Luau::CompileOptions { };
    compileoptions.optimizationLevel = 1;
    compileoptions.debugLevel = 1;
    compileoptions.vectorLib = "Vector3";
    compileoptions.vectorCtor = "new";
    compileoptions.vectorType = "Vector3";
    compileoptions.mutableGlobals = mutableglobals;
    std::string bytecode = Luau::compile(code, compileoptions, {}, &encoder);
    return bytecode;
}


int execution::load_string(lua_State* L, std::string chunk_name, std::string code) {
    std::string bytecode = this->compile(code);

    if (luau_load(L, chunk_name.c_str(), bytecode.data(), bytecode.size(), 0) != LUA_OK) {
        lua_pushnil(L);
        lua_pushvalue(L, -2);
        return 2;
    }

    Closure* cl = clvalue(luaA_toobject(L, -1));
    set_capabilities(cl->l.p, &capabilities);

    lua_setsafeenv(L, LUA_GLOBALSINDEX, false);

    return 1;
}


bool execution::run_code(lua_State* state, std::string code) {
    std::string bytecode = this->compile(code);

    if (bytecode.at(0) == 0) {
        const char* error = bytecode.c_str() + 1;
        rbx::standard_out::printf(rbx::message_type::message_error, error);
        return false;
    }
    else {
        lua_State* L = lua_newthread(state);
        luaL_sandboxthread(L);
        lua_pop(state, 1);

        L->userdata->identity = 8;
        L->userdata->capabilities = capabilities;

        lua_pushcclosure(L, reinterpret_cast<lua_CFunction>(rbx::script_context::task_defer), "defer", 0);

        if (luau_load(L, "", bytecode.data(), bytecode.size(), 0) != LUA_OK) {
            rbx::standard_out::printf(rbx::message_type::message_error, "unknown error occurred.");
            return false;
        }

        Closure* cl = clvalue(luaA_toobject(L, -1));

        set_capabilities(cl->l.p, &capabilities);

        lua_pcall(L, 1, 0, 0);

        return true;
    }
}
