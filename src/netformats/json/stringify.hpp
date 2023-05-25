/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/json/basic_array.hpp>
#include <netformats/json/basic_object.hpp>
#include <netformats/json/basic_value.hpp>

#include <netformats/unicode_tokenizer.hpp>

#include <string>

namespace netformats::json{

    enum class stringify_error{
        string_not_utf8
    };

    template <typename T>
    struct stringifier;

    template <typename Allocator>
    struct stringifier<std::basic_string<char, std::char_traits<char>, Allocator>>{

        using string_type = std::basic_string<char, std::char_traits<char>, Allocator>;

        static inline string_type to_string(const string_type& input){
            string_type result{input.get_allocator()};
            result.reserve(input.size() + 2);

            unicode::tokenizer tok{input};

            result.push_back('\"');

            auto expected_character = tok.consume_one();
            if(expected_character.)


        }

    };

}
