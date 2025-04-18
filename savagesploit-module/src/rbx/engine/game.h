//
// Created by savage on 17.04.2025.
//

#pragma once
#include "game_structures.h"

template <typename T>
inline T rebase(uintptr_t rva) {
    return rva != NULL ? (T)(rva + reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr))) : (T)(NULL);
};

namespace rbx {
    namespace rvas {

        namespace standard_out {
            inline const uintptr_t printf = 0x1644c90;
        }

        namespace taskscheduler {
            inline const uintptr_t taskschedulermk2 = 0x6375110;
            inline const uintptr_t fps_limit = rebase<uintptr_t>(0x5AD2768); // remove this when we have setfflag
        }

        namespace script_context {
            inline const uintptr_t get_global_state = 0xef84a0;
            inline const uintptr_t decrypt_state = 0xc9fd00;

            inline const uintptr_t task_defer = 0x10dd100;
        }

        namespace luau {
            inline const uintptr_t luau_execute = 0x26DB300;
            inline const uintptr_t luah_dummynode = 0x463ED28;
            inline const uintptr_t luao_nilobject = 0x463f5f8;
            inline const uintptr_t luac_step = 0x26a30d0;
            inline const uintptr_t opcode_table_lookup = 0x509D270;
        }

    }

    namespace offsets {

        namespace taskscheduler {
            inline uintptr_t job_start = 0x1d0;
            inline uintptr_t job_end = 0x1d8;
            inline uintptr_t job_name = 0x18;

            inline uintptr_t max_fps = 0x1B0;
        }

        namespace instance {
            inline const uintptr_t class_descriptor = 0x18;
            inline const uintptr_t parent = 0x50;
            inline const uintptr_t name = 0x78;
            inline const uintptr_t children = 0x80;
        }

        namespace luagc {
            inline const uintptr_t script_context = 0x1e8;
        }

        namespace script_context {
            inline const uintptr_t global_state = 0x138;
            inline const uintptr_t decrypt_state = 0x88;
        }

    }

    namespace standard_out {
        inline auto printf = rebase<void(__fastcall*)(int32_t, const char*, ...)>(rvas::standard_out::printf);
    }

    namespace script_context {
        inline auto decrypt_state = rebase<lua_State*(__fastcall*)(uintptr_t)>(rvas::script_context::decrypt_state);
        inline auto get_global_state = rebase<uintptr_t(__fastcall*)(uintptr_t, uintptr_t*, uintptr_t*)>(rvas::script_context::get_global_state);

        inline auto task_defer = rebase<__int64(__fastcall*)(lua_State*)>(rvas::script_context::task_defer);
    }

    namespace luau {
        inline auto opcode_table_lookup = rebase<BYTE*>(rvas::luau::opcode_table_lookup);
        inline auto luah_dummynode = rebase<uintptr_t>(rvas::luau::luah_dummynode);
        inline auto luao_nilobject = rebase<uintptr_t>(rvas::luau::luao_nilobject);
        inline auto luau_execute = rebase<void(__fastcall*)(lua_State*)>(rvas::luau::luau_execute);
        inline auto luac_step = rebase<size_t(__fastcall*)(lua_State*, bool assist)>(rvas::luau::luac_step);
    }

}