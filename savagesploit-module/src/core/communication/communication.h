//
// Created by savage on 21.04.2025.
//

#pragma once
#include <memory>


class communication {
public:
    void initialize();
};

inline const auto g_communication = std::make_unique<communication>();