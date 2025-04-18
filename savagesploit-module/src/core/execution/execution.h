//
// Created by savage on 17.04.2025.
//

#pragma once
#include <string>
#include "lua.h"
#include "Luau/Bytecode.h"
#include "Luau/BytecodeBuilder.h"
#include "Luau/BytecodeUtils.h"
#include "src/rbx/engine/game.h"

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

class execution {
public:
    uintptr_t capabilities = 0xFFFFFFFFFFFFFFFF;

    bool run_code(lua_State*, std::string);
};

inline const auto g_execution = std::make_unique<execution>();