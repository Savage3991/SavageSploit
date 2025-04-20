//
// Created by savage on 17.04.2025.
//

#pragma once

#include <memory>
#include <Windows.h>

class entry_point {
public:
    inline void entry(HMODULE DllModule);
};

inline const auto g_entry_point = std::make_unique<entry_point>();