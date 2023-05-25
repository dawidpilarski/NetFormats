/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <initializer_list>

namespace netformats::json::details{

    constexpr char character_min = 0x20;

    constexpr char whitespace_space = 0x20;
    constexpr char whitespace_carriage_return = 0x0A;
    constexpr char whitespace_newline = 0x0D;
    constexpr char whitespace_vertical_tab = 0x09;

    constexpr auto whitespace_allowed = {whitespace_space, whitespace_carriage_return, whitespace_newline, whitespace_vertical_tab};

    constexpr char escape_quote = '"';
    constexpr char escape_backslash = '\\';
    constexpr char escape_slash = '/';
    constexpr char escape_backspace = 'b';
    constexpr char escape_form_feed = 'f';
    constexpr char escape_newline = 'n';
    constexpr char escape_carriage_return = 'r';
    constexpr char escape_vertical_tab = 't';

    constexpr char escape_hex_sequence = 'u';
    constexpr unsigned escape_hex_max_characters = 4;

    constexpr auto escape_allowed_characters = {
            escape_quote,
            escape_backslash,
            escape_slash,
            escape_backspace,
            escape_form_feed,
            escape_newline,
            escape_carriage_return,
            escape_vertical_tab,
            escape_hex_sequence
    };

}