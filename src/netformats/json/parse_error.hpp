/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <cstddef>
#include <netformats/text_position.hpp>
#include <netformats/unicode_tokenizer.hpp>

namespace netformats::json{

    enum class parse_error_reason{
        invalid_utf_8_encoding = static_cast<unsigned>(unicode::unicode_error::invalid_utf_encoding),
        utf_8_codepoint_out_of_range = static_cast<unsigned>(unicode::unicode_error::codepoint_out_of_range),

        integer_0_with_multiple_digits,
        integer_minus_without_digits,
        fraction_no_digits_after_dot,
        invalid_character_after_exponent,
        number_could_not_be_parsed,
        hex_invalid,
        escaped_character_invalid,
        string_missing_finishing_quote,
        invalid_character_typo,
        could_not_match_any_value_type,
        missing_colon_after_key,
        expected_element_after_key,
        expected_brace,
        expected_closing_brace,
        expected_closing_bracket,
        remaining_data_after_json_parse
    };

    [[nodiscard]] constexpr parse_error_reason parse_error_from_unicode_error(unicode::unicode_error err){
        return static_cast<parse_error_reason>(static_cast<int>(err));
    }

    [[nodiscard]] constexpr bool is_encoding_error(parse_error_reason reason){
        switch (reason) {
            case parse_error_reason::invalid_utf_8_encoding:
            case parse_error_reason::utf_8_codepoint_out_of_range:
                return true;
            default:
                return false;
        }
    }


    [[nodiscard]] constexpr std::string_view to_string_view(parse_error_reason error_reason){
        switch(error_reason){
            case parse_error_reason::invalid_utf_8_encoding:
                return "Invalid UTF-8 encoding. Encountered sequence of bytes, which cannot be decoded as UTF-8.";
            case parse_error_reason::utf_8_codepoint_out_of_range:
                return "UTF-8 codepoint out of range. Supported codepoints in json are: 0x0020-0x10FFFF, 0x000A, 0x000D, 0x0009.";
            case parse_error_reason::integer_0_with_multiple_digits:
                return "Invalid integer. Integers starting from digit 0 cannot be followed by other digits.";
            case parse_error_reason::integer_minus_without_digits:
                return R"(Invalid integer. Integer started with '-' sign, but not digits follow '-'.)";
            case parse_error_reason::fraction_no_digits_after_dot:
                return "Invalid faction part. Numbers with fraction part must contain digits after \'.\'.";
            case parse_error_reason::invalid_character_after_exponent:
                return R"(Invalid exponent in number. 'e'/'E' characters must be followed bu optional sign, and mandatory digits.)";
            case parse_error_reason::number_could_not_be_parsed:
                return "Could not create number out of the string. Check your conversion function.";
            case parse_error_reason::hex_invalid:
                return "Invalid hex character. Hex character needs to be in range a-z, or A-Z, or 0-9.";
            case parse_error_reason::escaped_character_invalid:
                return R"(Invalid escaped character. After '\' only limited characters are allowed [", \, /, b, f, n, r, t, u[hex,hex,hex,hex]].)";
            case parse_error_reason::string_missing_finishing_quote:
                return R"(Invalid string. When parsing string, ending '"' character was not found.)";
            case parse_error_reason::invalid_character_typo:
                return "Probable typo. Unexpected character while parsing one of following values: true, false, null.";
            case parse_error_reason::could_not_match_any_value_type:
                return "No value. Expected value, but could not parse any.";
            case parse_error_reason::missing_colon_after_key:
                return "Missing colon after key. Keys in object must be followed by \':\'.";
            case parse_error_reason::expected_element_after_key:
                return "Missing value after key. Object's key does not have any associated value.";
            case parse_error_reason::expected_brace:
                return "Redundant comma. Last element in object and array cannot be followed by comma.";
            case parse_error_reason::expected_closing_bracket:
                return "Invalid array. After parsing arrays elements, ending ']' character was not found.";
            case parse_error_reason::remaining_data_after_json_parse:
                return "Remaining data after parse. Json parsing finished, but there is still some data left.";
            case parse_error_reason::expected_closing_brace:
                return "Invalid object. After parsing objects members, ending '}' character was not found.";
        }

        assert(false);
        return "unknown";
    }

    template <typename Allocator = std::allocator<char>>
    [[nodiscard]] std::basic_string<char, std::char_traits<char>, Allocator> to_string(parse_error_reason error_reason, Allocator alloc = Allocator{}){
        return std::basic_string<char, std::char_traits<char>, Allocator> {to_string_view(error_reason), std::move(alloc)};
    }

    struct parse_error{
        text_position position;
        parse_error_reason reason;

        const char* buffer_iterator;
        std::string_view buffer;
    };


    namespace details{

        struct forward_codepoints_result{
            const char* destination;
            const char* closest_newline;
            std::size_t codepoints_to_newline;
        };

        inline forward_codepoints_result find_buffer_start(std::string_view buffer, std::size_t buffer_offset, unsigned codepoints_back){
            forward_codepoints_result result{};
            result.closest_newline = nullptr;
            result.codepoints_to_newline = 0;

            while(codepoints_back --> 0 && buffer_offset > 0){
                constexpr unsigned max_10_bytes_in_codepoint = 3;
                unsigned current_codepoint_byte = max_10_bytes_in_codepoint;
                while (current_codepoint_byte --> 0){ // codepoints before failure point are assumed to be correct unicode
                    std::byte current_byte = (buffer_offset == buffer.size()) ? std::byte{'\0'} : static_cast<std::byte>(buffer[buffer_offset]);
                    if(!unicode::starts_with_10(current_byte) || buffer_offset == 0){
                        break;
                    }
                    --buffer_offset;
                }


                //todo refactor
                auto current_byte = (buffer_offset == buffer.size()) ? std::byte{'\0'} : static_cast<std::byte>(buffer[buffer_offset]);
                if(static_cast<char>(current_byte) == '\n' && result.closest_newline == nullptr){
                    result.closest_newline = buffer.data() + buffer_offset;
                }

                if(result.closest_newline == nullptr){
                    ++result.codepoints_to_newline;
                }

                --buffer_offset;
                assert(!unicode::starts_with_10(current_byte));
            }

            result.destination = buffer.data() + buffer_offset;
            return result;
        }

        inline forward_codepoints_result find_buffer_end(std::string_view buffer, std::size_t buffer_offset, unsigned codepoints_forward) {
            forward_codepoints_result result{};
            result.closest_newline = nullptr;
            auto new_buffer = std::string_view{buffer.data()+buffer_offset, buffer.data() + buffer.size()};

            unicode::tokenizer tok{new_buffer};

            while(codepoints_forward --> 0){
                if(!tok.consume_one()){
                    break;
                }
                if(result.closest_newline == nullptr && *tok.current_iterator() == '\n'){
                   result.closest_newline = tok.current_iterator();
                }
            }

            result.destination = tok.current_iterator();
            return result;
        }
    }


    namespace details{
        template <typename string>
        std::size_t escape_whitespaces(string& characters){
            std::remove_reference_t<string> result{characters.get_allocator()};
            result.reserve(characters.capacity());

            std::size_t number_of_whitespaces = 0;
            for(auto character : characters){
                switch (character) {
                    case '\n':
                        result.append("\\n");
                        number_of_whitespaces++;
                        break;
                    case '\t':
                        result.append("\\t");
                        number_of_whitespaces++;
                        break;
                    case '\r':
                        result.append("\\r");
                        number_of_whitespaces++;
                        break;
                    case '\f':
                        result.append("\\r");
                        number_of_whitespaces++;
                        break;
                    case '\v':
                        result.append("\\v");
                        number_of_whitespaces++;
                        break;
                    default:
                        result.push_back(character);
                }
            }

            characters = std::move(result);

            return number_of_whitespaces;
        }
    }

    template <typename Allocator = std::allocator<char>>
    std::basic_string<char, std::char_traits<char>, Allocator> to_string(const parse_error& err, Allocator alloc = Allocator{}){
        using string = std::basic_string<char, std::char_traits<char>, Allocator>;

        string str{alloc};

        str.append("Parsing failed at position [line:column] " + to_string(err.position, alloc));
        str.append("\nReason: " + to_string(err.reason, alloc));
        str.append("\n\n");

        if(err.reason <= parse_error_reason::utf_8_codepoint_out_of_range){
            return str;
        }

        constexpr unsigned max_codepoints_context = 20;
        constexpr char underscore_character = '~';
        constexpr char point_character = '^';

        auto buffer_offset = std::distance(err.buffer.begin(), err.buffer_iterator);

        details::forward_codepoints_result before_codepoints = details::find_buffer_start(err.buffer, buffer_offset, max_codepoints_context);
        details::forward_codepoints_result after_codepoints = details::find_buffer_end(err.buffer, buffer_offset, max_codepoints_context);

        const char* const error_start = (before_codepoints.closest_newline == nullptr) ? before_codepoints.destination : before_codepoints.closest_newline+1;
        if(before_codepoints.closest_newline != nullptr) before_codepoints.codepoints_to_newline--;

        const char* const error_end = (after_codepoints.closest_newline == nullptr) ? after_codepoints.destination : after_codepoints.closest_newline;

        string error {error_start, error_end, alloc};
        std::size_t number_of_whitespaces_without_space = details::escape_whitespaces(error);
        string error_marker {alloc};

        str.append(error);
        str.push_back('\n');

        auto before_error_codepoints = before_codepoints.codepoints_to_newline;

        //each whitespace except space is substituted with \\ + character
        before_error_codepoints += number_of_whitespaces_without_space;

        while(before_error_codepoints -- > 0){
            if(before_error_codepoints < 3){
                error_marker += underscore_character;
            } else {
                error_marker += ' ';
            }
        }

        error_marker += point_character;
        error_marker += string(3, underscore_character, alloc);

        str.append(error_marker);

        return str;
    }

}
