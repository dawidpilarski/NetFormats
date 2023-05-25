/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/unicode_tokenizer.hpp>
#include <netformats/json/constants.hpp>

#include <utility>
#include <ranges>

namespace netformats::json::details {
    enum class tokenization_error{
        invalid_utf_encoding = std::to_underlying(unicode::unicode_error::invalid_utf_encoding),
        codepoint_out_of_range
    };

    bool operator==(tokenization_error json_error, unicode::unicode_error unicode_error){
        return std::to_underlying(json_error) == std::to_underlying(unicode_error);
    }

    tokenization_error tokenization_error_from_unicode_error(unicode::unicode_error error){
        return static_cast<tokenization_error>(std::to_underlying(error));
    }

    class tokenizer : private unicode::tokenizer{
    public:
        using unicode::tokenizer::tokenizer;

        using unicode::tokenizer::character;
        using unicode::tokenizer::source_position;
        using unicode::tokenizer::current_buffer_iterator;
        using unicode::tokenizer::source_buffer;

        [[nodiscard]] constexpr expected<std::optional<char32_t>, tokenization_error> peek_next(){
            auto expected_character = unicode::tokenizer::peek_next();

            if(expected_character.has_value()){
                auto character = expected_character.value();
                if(character < character_min){
                    if(std::ranges::none_of(whitespace_allowed, [character](auto val){return val == character;})){
                        return unexpected{tokenization_error::codepoint_out_of_range};
                    }
                }

                return character;
            }

            assert(expected_character.error() == tokenization_error::invalid_utf_encoding);
            return unexpected{tokenization_error_from_unicode_error(expected_character.error())};
        }

        [[nodiscard]] constexpr expected_no_value<tokenization_error> consume_one(){
            auto expected_next = peek_next();
            if(!expected_next.has_value()){
                return unexpected{expected_next.error()};
            }

            (void) unicode::tokenizer::consume_one();

            return {};
        }

    };

}