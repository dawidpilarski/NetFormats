/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once
#include <string>
#include <cmath>
#include <charconv>

struct text_position{
    std::size_t line;
    std::size_t column;

    auto operator<=>(const text_position&) const = default;
};

template <typename Allocator = std::allocator<char>>
[[nodiscard]] inline std::basic_string<char, std::char_traits<char>, Allocator> to_string(text_position pos, Allocator alloc = std::allocator<char>{}){
    auto number_of_chars = [](std::size_t number){
        return static_cast<std::size_t>(log10(static_cast<double>(number)) + 1);
    };

    const auto number_of_col_chars = number_of_chars(pos.column);
    const auto number_of_line_chars = number_of_chars(pos.line);

    std::basic_string<char, std::char_traits<char>, Allocator> string{std::move(alloc)};
    string.resize(number_of_line_chars + number_of_col_chars + 1);

    std::to_chars_result result = std::to_chars(string.data(), string.data()+number_of_line_chars, pos.line);
    assert(result.ec == std::errc{});

    string[number_of_line_chars] = ':';

    auto line_ptr = string.data() + number_of_line_chars+1;
    auto line_ptr_end = string.data() + string.size();

    result = std::to_chars(line_ptr, line_ptr_end, pos.column);
    assert(result.ec == std::errc{});

    return string;
}