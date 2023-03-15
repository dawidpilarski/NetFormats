/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/text_position.hpp>
#include <netformats/expected.hpp>

#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <optional>
#include <algorithm>
#include <limits>

namespace netformats::unicode {

    enum class unicode_error{
        invalid_utf_encoding,
        codepoint_out_of_range,
        end_of_input
    };

    constexpr expected<unsigned, unicode_error> to_character_size(char firstByte) {
        const std::bitset<8> value{static_cast<uint8_t>(firstByte)};
        if (value[7] == 0) return 1;
        else if (value[6] == 0) [[unlikely]] return unexpected{unicode_error::invalid_utf_encoding};
        else if (value[5] == 0) return 2;
        else if (value[4] == 0) return 3;
        else if (value[3] == 0) return 4;
        else [[unlikely]] return unexpected(unicode_error::invalid_utf_encoding);
    }

    constexpr bool starts_with_10(std::byte byte) {
        byte &= std::byte{0b11000000};
        return byte == std::byte{0b10000000};
    };

    constexpr expected<unsigned, unicode_error> to_character_size(char32_t unicodeChar) {

        constexpr unsigned mask = 0xFF000000;
        for (unsigned int i = 0; i < 4; ++i, unicodeChar <<= 8) {
            if ((unicodeChar & mask) != 0) return to_character_size(static_cast<char>(unicodeChar >> 24));
        }

        return 1;
    }

    constexpr char32_t as_character(char const *begin, unsigned size) {
        char32_t new_character = 0;
        while (size-- > 0) {
            new_character <<= 8;
            new_character |= static_cast<unsigned char>(*begin);
            begin++;
        }

        return new_character;
    }

    //todo use high performance algorithms for encoding and decoding
    constexpr expected<char32_t, unicode_error> codepoint_to_character(char32_t codepoint){

        uint32_t result{};
        if(codepoint <= 0x7F) return codepoint;

        else if (codepoint <= 0x07FF){
            uint_fast8_t  youngest = codepoint & 0b00111111;
            codepoint >>= 6;
            uint_fast8_t oldest = codepoint & 0b00011111;
            result |= 0xC0u | oldest; //110 start
            result <<= 8;
            result |= 0x80u | youngest;
            return result;
        } else if (codepoint <= 0xFFFF){
            uint_fast8_t  youngest = codepoint & 0b00111111;
            codepoint >>= 6;
            uint_fast8_t middle = codepoint & 0b00111111;
            codepoint >>= 6;
            uint_fast8_t oldest = codepoint & 0b00001111;
            result |= 0xE0u | oldest; //1110 start
            result <<= 8;
            result |= 0x80u | middle;
            result <<= 8;
            result |= 0x80u | youngest;
            return result;
        } else if (codepoint <= 0x10FFFF){
            uint8_t  youngest = codepoint & 0b00111111;
            codepoint >>= 6;
            uint8_t middle_younger = codepoint & 0b00111111;
            codepoint >>= 6;
            uint8_t middle_older = codepoint & 0b00111111;
            codepoint >>= 6;
            uint8_t oldest = codepoint & 0b00000111;
            result |= 0xF0u | oldest; //1110 start
            result <<= 8;
            result |= 0x80u | middle_older;
            result <<= 8;
            result |= 0x80u | middle_younger;
            result <<= 8;
            result |= 0x80u | youngest;
            return result;
        }

        [[unlikely]] return unexpected{unicode_error::codepoint_out_of_range};
    }

    //todo handle CRLF
    class tokenizer {
    public:

        //todo test empty input
        constexpr explicit tokenizer(std::string_view input) : buffer(input) {}

        [[nodiscard]] constexpr std::optional<char32_t> character() {
            if (buffer[current.idx] == '\0') [[unlikely]] return {};
            return current.parsed_character;
        }

        [[nodiscard]] text_position source_position() const {
            return {.line = current.line_number, .column = current.col_number};
        }

        [[nodiscard]] std::string next_source_position() const {
            return std::to_string(next->line_number) + ":" + std::to_string(next->col_number);
        }

        [[nodiscard]] constexpr expected<std::optional<char32_t>, unicode_error> peek_next() {
            if (next) return next->parsed_character;

            expected<std::optional<std::pair<unsigned, char32_t>>, unicode_error> expectedCharacter = read_character(
                    current.idx + current.parsed_character_size);
            if(!expectedCharacter){
                return unexpected{std::move(expectedCharacter).error()};
            }

            auto& character = *expectedCharacter;
            if (!character) return std::nullopt;

            unsigned long next_line_number = current.line_number;
            unsigned long next_col_number = current.col_number;
            if (character->second == '\n') {
                next_col_number = 0;
                next_line_number++;
            } else {
                next_col_number++;
            }

            next = position{current.idx + current.parsed_character_size,
                            next_line_number,
                            next_col_number,
                            character->second,
                            character->first};


            return next->parsed_character;
        }

        [[nodiscard]] char const *current_iterator() const {
            return buffer.data() + current.idx;
        }

        [[nodiscard]] constexpr expected_no_value<unicode_error> consume_one() {
            if(current.idx >= buffer.size())  return {};

            if (next) {
                current = *next;
                next = {};
                return {};
            }

            current.idx = current.idx + current.parsed_character_size;
            expected expectedNewCharacter = read_character(current.idx);
            if(!expectedNewCharacter){
                return unexpected{std::move(expectedNewCharacter).error()};
            }
            if (!*expectedNewCharacter) [[unlikely]] {
                current.parsed_character = '\0';
                current.parsed_character_size = 1;
                return {};
            }

            auto& newCharacter = *expectedNewCharacter;
            current.parsed_character_size = newCharacter->first;
            current.parsed_character = newCharacter->second;

            if (current.parsed_character == '\n') {
                current.line_number++;
                current.col_number = 1;
            } else {
                current.col_number++;
            }

            return {};
        }

        [[nodiscard]] constexpr std::string_view source_buffer() const {
            return buffer;
        }

    private:
        struct position {
            std::size_t idx = 0;
            unsigned long line_number = 1;
            unsigned long col_number = 0;
            char32_t parsed_character = '\0';
            unsigned parsed_character_size = 0;
        };
        position current;
        std::optional<position> next;

        [[nodiscard]] constexpr expected<std::optional<std::pair<unsigned, char32_t>>, unicode_error> read_character(std::size_t idx) {
            if(idx >= buffer.size()) [[unlikely]] {
                return std::nullopt;
            }

            std::pair<unsigned, char32_t> result;
            expected expectedSize = unicode::to_character_size(buffer[idx]);
            if(!expectedSize){
                return unexpected{expectedSize.error()};
            }
            assert(*expectedSize > 0);
            result.first = *expectedSize;

            if (idx + *expectedSize > buffer.size()){
                return unexpected{unicode_error::invalid_utf_encoding};
            }
            if (*expectedSize > 1 && std::any_of(buffer.data() + idx + 1, buffer.data() + idx + *expectedSize,
                            [](auto element) { return !unicode::starts_with_10(static_cast<std::byte>(element)); })) [[unlikely]] {
                return unexpected{unicode_error::invalid_utf_encoding};
            }

            constexpr char32_t max_character = *codepoint_to_character(0x10FFFF);
            static_assert(max_character == 0xf48fbfbf);

            result.second = unicode::as_character(buffer.data() + idx, *expectedSize);

            if (result.second < 0x0020 || result.second > max_character) {
                switch (result.second) {
                    case ' ':
                    case '\r':
                    case '\n':
                    case '\t':
                    return result;
                }
                return unexpected{unicode_error::codepoint_out_of_range};
            }

            return result;
        }

        std::string_view buffer{};
    };
}
