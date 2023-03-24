/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

struct text_position{
    std::size_t line;
    std::size_t column;

    auto operator<=>(const text_position&) const = default;
};

//todo allocator
template <typename string>
[[nodiscard]] inline string to_string(text_position pos){
    return std::to_string(pos.column) + ":" + std::to_string(pos.line);
}