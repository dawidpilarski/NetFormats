/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/json/basic_array.hpp>
#include <netformats/json/basic_object.hpp>
#include <netformats/json/basic_value.hpp>
#include <netformats/json/constants.hpp>

#include <netformats/unicode_tokenizer.hpp>

#include <string>
#include <array>

namespace netformats::json{

    enum class stringify_error{
        string_not_utf8
    };

    template <typename T>
    struct stringifier;

    template <typename Allocator>
    struct stringifier<std::basic_string<char, std::char_traits<char>, Allocator>>{

        using string_type = std::basic_string<char, std::char_traits<char>, Allocator>;

        static inline expected<string_type, stringify_error> to_string(const string_type& input){
            string_type result{input.get_allocator()};
            result.reserve(input.size() + 2);

            unicode::tokenizer tok{input};

            result.push_back('\"');

            auto expected_character = tok.peek_next();
            if(!expected_character.has_value()){
                return unexpected{stringify_error::string_not_utf8};
            }

            std::optional character_or_eof = *expected_character;

            while(character_or_eof.has_value()){
                auto character = *character_or_eof;

                if(character < details::character_min){
                    std::array<char, 2> buff;
                    [[maybe_unused]] std::to_chars_result conversion_result = std::to_chars(buff.begin(), buff.end(), character, 16);
                    assert(conversion_result.ec == std::errc{});

                    result.append("\\u00");
                    result.append(std::string_view{buff.data(), buff.size()});
                } else {
                    unicode::copy(character, std::back_inserter(result));
                }

                (void)tok.consume_one();

                auto new_expected_character = tok.peek_next();
                if(!new_expected_character.has_value()){
                    return unexpected{stringify_error::string_not_utf8};
                }

                character_or_eof = *new_expected_character;
            }

            result.push_back('\"');
        }
    };

}
