/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once
#include <netformats/json/basic_value.hpp>
#include <netformats/json/basic_object.hpp>
#include <netformats/json/basic_array.hpp>
#include <netformats/json/parse_error.hpp>
#include <netformats/json/config.hpp>
#include <netformats/unicode_tokenizer.hpp>
#include <netformats/expected.hpp>
#include <netformats/json/tokenizer.hpp>
#include <charconv>

#include <array>
#include <system_error>
#include <charconv>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <iterator>

/*
 * object
    array
    string
    number
    "true"
    "false"
    "null"
 */

namespace netformats::json {

    template <typename Integer, typename Alloc>
    Integer create_integer(const char* begin, const char* end, [[maybe_unused]] Alloc allocator){
        Integer integer;
        auto result = std::from_chars(begin, end, integer);
        if(result.ec == std::errc{}){
            return integer;
        } else[[unlikely]]{
            throw std::runtime_error("Could not parse integer");
        }
    }

template <netformats::details::json_config config>
class basic_parser{
public:
    using null = typename config::null;
    using boolean = typename config::boolean;
    using floating_point = typename config::floating_point;
    using integer = typename config::integer;
    using string = typename config::string;
    template <typename T>
    using allocator = typename config::template allocator<T>;

    using value = basic_value<config>;
    using object = typename value::object;
    using array = typename value::array;
    using error = parse_error;
private:
    template <typename T>
    using expected = expected<T, error>;
    using expected_no_value = expected<null>;
public:

    explicit basic_parser(allocator<null> alloc) : root_allocator(std::move(alloc)){}
    basic_parser() : basic_parser(allocator<null>{}){}


    [[nodiscard]] expected<value> parse(std::string_view json){
        unicode::tokenizer tokenizer(json);
        expected<value> expected_element =  consume_element(tokenizer);

        if(!expected_element) return expected_element;

        auto expected_character = tokenizer.peek_next();
        if(expected_character && expected_character->has_value()){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::remaining_data_after_json_parse)};
        }

        return expected_element;
    }

    allocator<null> get_allocator(){return root_allocator;}

private:
    allocator<null> root_allocator;


    constexpr static inline error create_parse_error(unicode::tokenizer &tokenizer, parse_error_reason reason){
        error error;

        error.position = tokenizer.source_position();
        error.reason = reason;
        error.buffer = tokenizer.source_buffer();
        error.buffer_iterator = tokenizer.current_buffer_iterator();


        return error;
    }

    constexpr static inline error create_parse_error(unicode::tokenizer &tokenizer, unicode::unicode_error reason){
        error error;

        error.position = tokenizer.source_position();
        error.reason = parse_error_from_unicode_error(reason);
        error.buffer = tokenizer.source_buffer();
        error.buffer_iterator = tokenizer.current_buffer_iterator() + 1;

        return error;
    }

    template <typename T>
    unexpected<error> forward_error(expected<T>&& error){
        return unexpected{std::move(error.error())};
    }

    [[nodiscard]] constexpr expected_no_value consume_whitespace(unicode::tokenizer &tokenizer) {
        auto whitespace_characters = {u8'\u0020', u8'\u000A', u8'\u000D', u8'\u0009'};

        auto is_whitespace = [&whitespace_characters](char32_t character) {
            return std::any_of(whitespace_characters.begin(), whitespace_characters.end(), [&character](auto element) {
                return character == element;
            });
        };

        while (true) {
            auto expected_next_character = tokenizer.peek_next();
            if(!expected_next_character){
                return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
            }

            auto& next_character = *expected_next_character;
            if(!next_character) return {};

            if(!is_whitespace(next_character.value_or('A'))) return {};
            auto expected_no_error = tokenizer.consume_one();
            assert(expected_no_error);
        }

        assert(false);
    }

    enum class sign {
        plus = 1, minus = -1
    };

    [[nodiscard]] constexpr expected<sign> consume_sign(unicode::tokenizer &tokenizer) {
        auto expected_character = tokenizer.peek_next();
        if(!expected_character) return unexpected{create_parse_error(tokenizer, expected_character.error())};

        auto& character = *expected_character;
        if (!character.has_value()) return sign::plus;

        if (*character == '+') {
            (void)tokenizer.consume_one();
            return sign::plus;
        }

        if (*character == '-') {
            (void)tokenizer.consume_one();
            return sign::minus;
        }

        return sign::plus;
    }

    [[nodiscard]] constexpr expected<std::optional<char>> consume_onenine(unicode::tokenizer &tokenizer) {
        auto expectedCharacter = tokenizer.peek_next();
        if(!expectedCharacter){
            return unexpected{create_parse_error(tokenizer, expectedCharacter.error())};
        }

        auto& character = *expectedCharacter;

        if (!character.has_value()) return {};

        if (*character >= '1' && *character <= '9') {
            (void)tokenizer.consume_one();
            return static_cast<char>(*character);
        }

        return {};
    }

    [[nodiscard]] constexpr expected<std::optional<char>> consume_digit(unicode::tokenizer &tokenizer) {
        auto expected_character = tokenizer.peek_next();
        if(!expected_character){
            return unexpected{create_parse_error(tokenizer, expected_character.error())};
        }

        auto& character = *expected_character;

        if (!character.has_value()) return {};

        if (*character == '0') {
            (void)tokenizer.consume_one();
            return '0';
        }

        return consume_onenine(tokenizer);
    }

    [[nodiscard]] constexpr expected<std::optional<std::string_view>> consume_digits(unicode::tokenizer &tokenizer) {
        std::optional<char> digit;

        const char *begin = nullptr;
        const char *end = nullptr;
        auto expected_result = consume_digit(tokenizer);

        if(!expected_result){
            return forward_error(std::move(expected_result));
        }

        auto& result = *expected_result;

        if (result) {
            begin = tokenizer.current_buffer_iterator();
            while (true){
                auto expected_digit = consume_digit(tokenizer);
                if(!expected_digit) return forward_error(std::move(expected_digit));
                if(!*expected_digit) break;
            }
            end = tokenizer.current_buffer_iterator() + 1;

            return std::string_view{begin, end};
        }

        return {};
    }

    [[nodiscard]] expected<std::optional<const char *>> consume_integer(unicode::tokenizer &tokenizer) {
        signed multiplier = 1;
        const char *begin = nullptr;
        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }

        auto& next_character = *expected_next_character;

        if (!next_character) return {};
        if (*next_character == '-') {
            multiplier = -1;
            begin = tokenizer.current_buffer_iterator();
            (void)tokenizer.consume_one();
        }

        auto expected_zero = tokenizer.peek_next();
        if(!expected_zero){
            return unexpected{create_parse_error(tokenizer, expected_zero.error())};
        }
        if (*expected_zero == '0') {
            (void) tokenizer.consume_one();

            if (!begin) begin = tokenizer.current_buffer_iterator();
            if (auto expected_digit = consume_digit(tokenizer); *expected_digit && expected_digit->has_value()) {
                return unexpected{create_parse_error(tokenizer, parse_error_reason::integer_0_with_multiple_digits)};
            }

            return begin;
        }

        auto expected_first_onenine  = consume_onenine(tokenizer);
        if(!expected_first_onenine){
            return forward_error(std::move(expected_first_onenine));
        }
        auto& first_one_nine = *expected_first_onenine;

        if (!first_one_nine && multiplier == -1) {
            return unexpected{create_parse_error(tokenizer, parse_error_reason::integer_minus_without_digits)};
        }

        if (!first_one_nine) {
            return {};
        }

        if (!begin) begin = tokenizer.current_buffer_iterator();

        auto expected_digits = consume_digits(tokenizer);
        if(!expected_digits)
            return forward_error(std::move(expected_digits));

        return begin;
    }

    [[nodiscard]] expected<std::optional<std::string_view>> consume_fraction(unicode::tokenizer &tokenizer) {
        auto expected_dot = tokenizer.peek_next();
        if(!expected_dot){
            return unexpected{create_parse_error(tokenizer, expected_dot.error())};
        }

        if (*expected_dot != '.') return {};

        (void) tokenizer.consume_one();

        const char *begin = tokenizer.current_buffer_iterator();

        auto expected_digits = consume_digits(tokenizer);
        if(!expected_digits){
            return forward_error(std::move(expected_digits));
        }

        if (!*expected_digits){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::fraction_no_digits_after_dot)};
        }

        const auto &parsed_digits = **expected_digits;

        return std::string_view{begin, parsed_digits.data() + parsed_digits.size()};
    }

    [[nodiscard]] expected<std::optional<std::string_view>> consume_exponent(unicode::tokenizer &tokenizer) {
        auto expected_next = tokenizer.peek_next();
        if(!expected_next){
            return unexpected{create_parse_error(tokenizer, expected_next.error())};
        }

        auto& next = *expected_next;

        if (!next) return {};
        if (*next != 'e' && *next != 'E') return {};

        (void) tokenizer.consume_one();
        const char *begin = tokenizer.current_buffer_iterator();

        if(auto expected_sign = consume_sign(tokenizer); !expected_sign){
            return forward_error(std::move(expected_sign));
        }

        auto expected_digits = consume_digits(tokenizer);
        if(!expected_digits){
            return forward_error(std::move(expected_digits));
        }

        const auto& consumed_digits = *expected_digits;
        if (!consumed_digits){
            (void)tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::invalid_character_after_exponent)};
        }

        return std::string_view{begin, consumed_digits->data() + consumed_digits->size()};
    }

    [[nodiscard]] expected<std::optional<std::variant<integer, floating_point>>> consume_number(unicode::tokenizer &tokenizer) {
        auto expected_integer = consume_integer(tokenizer);
        if(!expected_integer){
            return forward_error(std::move(expected_integer));
        }
        const auto& consumed_integer = *expected_integer;
        if (!consumed_integer) return {};

        auto expected_fraction = consume_fraction(tokenizer);
        if(!expected_fraction){
            return forward_error(std::move(expected_fraction));
        }
        const auto& consumed_fraction = *expected_fraction;
        auto expected_exponent = consume_exponent(tokenizer);
        if(!expected_exponent){
            return forward_error(std::move(expected_exponent));
        }
        const auto& consumed_exponent = *expected_exponent;

        if (!consumed_fraction && !consumed_exponent) {
            return create_integer<integer>(*consumed_integer, tokenizer.current_buffer_iterator() + 1, get_allocator());
        }

        long double number;
        const char *begin = *consumed_integer;
        auto result = std::from_chars(begin, tokenizer.current_buffer_iterator() + 1, number);

        if (result.ec != std::errc{}) {
            return unexpected{create_parse_error(tokenizer, parse_error_reason::number_could_not_be_parsed)};
        }

        return number;
    }

    [[nodiscard]] expected<std::optional<char>> consume_hex(unicode::tokenizer &tokenizer) {
        auto expected_character = tokenizer.peek_next();
        if(!expected_character){
            return unexpected{create_parse_error(tokenizer, expected_character.error())};
        }
        auto& next_character = *expected_character;
        if (!next_character) return {};
        if ((*next_character >= 'A' && *next_character <= 'F') ||
            *next_character >= 'a' && *next_character <= 'f') {
            (void)tokenizer.consume_one();
            return static_cast<char>(*next_character);
        }

        auto digit = consume_digit(tokenizer);
        if(!digit){
            return forward_error(std::move(digit));
        }
        if (*digit) {
            return **digit;
        }

        return {};
    }

    [[nodiscard]] expected<std::optional<char32_t>> consume_escaped(unicode::tokenizer &tokenizer) {
        auto escaped_characters = {
                '\"',
                '\\',
                '/',
                'b',
                'f',
                'n',
                'r',
                't'
        };

        auto expected_character = tokenizer.peek_next();
        if(!expected_character){
            return unexpected{create_parse_error(tokenizer, expected_character.error())};
        }

        auto& next_character = *expected_character;
        if (!next_character) return {};
        if (std::any_of(escaped_characters.begin(), escaped_characters.end(), [&next_character](char character) {
            return *next_character == character;
        })) {
            (void) tokenizer.consume_one();
            switch (*next_character) {
                case 'b': return '\b';
                case 'f': return '\f';
                case 'n': return '\n';
                case 'r': return '\r';
                case 't': return '\t';
            }
            return *next_character;
        }

        if (*next_character != 'u') {
            return {};
        }

        (void) tokenizer.consume_one();

        auto first_hex_iter = tokenizer.current_buffer_iterator();
        std::array<std::optional<char>, 4> four_hexes = {0};

        if(auto expected_hex = consume_hex(tokenizer); !expected_hex ){
            return forward_error(std::move(expected_hex));
        } else{
            four_hexes[0] = *expected_hex;
        }
        if(auto expected_hex = consume_hex(tokenizer); !expected_hex ){
            return forward_error(std::move(expected_hex));
        } else{
            four_hexes[1] = *expected_hex;
        }
        if(auto expected_hex = consume_hex(tokenizer); !expected_hex ){
            return forward_error(std::move(expected_hex));
        } else{
            four_hexes[2] = *expected_hex;
        }
        if(auto expected_hex = consume_hex(tokenizer); !expected_hex ){
            return forward_error(std::move(expected_hex));
        } else{
            four_hexes[3] = *expected_hex;
        }

        if (std::any_of(std::begin(four_hexes), std::end(four_hexes), [](const auto &element) {
            return !element.has_value();
        })) {
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::hex_invalid)};
        }

        ++first_hex_iter;

        uint32_t character_from_hex;
        std::from_chars(first_hex_iter, tokenizer.current_buffer_iterator() + 1, character_from_hex, 16);
        auto conversion_result = unicode::codepoint_to_character(character_from_hex);
        if(!conversion_result){
            return unexpected{create_parse_error(tokenizer, conversion_result.error())};
        }
        return *conversion_result;
    }

    [[nodiscard]] expected<std::optional<char32_t>> consume_character(unicode::tokenizer &tokenizer) {
        auto expected_character = tokenizer.peek_next();
        if(!expected_character){
            return unexpected{create_parse_error(tokenizer, expected_character.error())};
        }

        auto& next_character = *expected_character;
        if (!next_character) return {};

        if (*next_character == '\"') return {};
        if (*next_character == '\\') {
            (void) tokenizer.consume_one();

            auto expected_consumed_character = consume_escaped(tokenizer);
            if(!expected_consumed_character){
                return forward_error(std::move(expected_consumed_character));
            }
            auto& consumed_character = *expected_consumed_character;

            if (!consumed_character){
                (void) tokenizer.consume_one();
                return unexpected{create_parse_error(tokenizer, parse_error_reason::escaped_character_invalid)};
            }

            return *consumed_character;
        }

        (void) tokenizer.consume_one();
        return *next_character;
    }

    [[nodiscard]] expected<std::optional<string>> consume_characters(unicode::tokenizer &tokenizer) {
        string characters{get_allocator()};

        while (true) {
            expected<std::optional<char32_t>> expected_character = consume_character(tokenizer);
            if(!expected_character){
                return forward_error(std::move(expected_character));
            }
            auto& character = *expected_character;

            if(!character) break;

            char32_t real_character = *character;
            const uint32_t mask = 0xFF000000;
            for (unsigned i = 0; i < 3; ++i) {
                uint8_t oldest_byte = (real_character & mask) >> 24;
                real_character <<= 8; //todo check if not UB

                if (oldest_byte != 0) characters.push_back(static_cast<char>(oldest_byte));
            }
            characters.push_back(static_cast<char>(real_character >> 24));
        }

        if (characters.empty()) return {};
        return characters;
    }

    [[nodiscard]] expected<std::optional<string>> consume_string(unicode::tokenizer &tokenizer) {
        auto expected_character = tokenizer.peek_next();
        if(!expected_character){
            return unexpected{create_parse_error(tokenizer, expected_character.error())};
        }
        std::optional<char32_t>& next_character = *expected_character;
        if (next_character != '\"') return {};

        (void) tokenizer.consume_one();

        auto expected_result = consume_characters(tokenizer);
        if(!expected_result){
            return forward_error(std::move(expected_result));
        }

        auto& result = *expected_result;

        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }

        next_character = *expected_next_character;
        if (next_character != '\"'){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::string_missing_finishing_quote)};
        }

        (void) tokenizer.consume_one();

        if (result.has_value()) return result;
        return string{get_allocator()};
    }

    [[nodiscard]] expected_no_value consume_character(unicode::tokenizer& tokenizer, char character){
        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }

        auto next_character = *expected_next_character;
        if(next_character != character) {
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::invalid_character_typo)};
        }

        (void) tokenizer.consume_one();

        return {};
    }

    [[nodiscard]] expected<std::optional<bool>> consume_boolean(unicode::tokenizer& tokenizer){
        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }
        auto next_character = *expected_next_character;
        if(!next_character) return {};

        if(*next_character == 't'){
            (void) tokenizer.consume_one();
            if(auto expected_no_error = consume_character(tokenizer, 'r'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            if(auto expected_no_error = consume_character(tokenizer, 'u'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            if(auto expected_no_error = consume_character(tokenizer, 'e'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return true;
        } else if(*next_character == 'f'){
            (void)tokenizer.consume_one();
            if(auto expected_no_error = consume_character(tokenizer, 'a'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            if(auto expected_no_error = consume_character(tokenizer, 'l'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            if(auto expected_no_error = consume_character(tokenizer, 's'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            if(auto expected_no_error = consume_character(tokenizer, 'e'); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return false;
        }

        return {};
    }

    [[nodiscard]] expected<std::optional<null>> consume_null(unicode::tokenizer& tokenizer){
        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }
        auto next_character = *expected_next_character;
        if(next_character != 'n') return {};
        (void) tokenizer.consume_one();
        if(auto expected_no_error = consume_character(tokenizer, 'u'); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }
        if(auto expected_no_error = consume_character(tokenizer, 'l'); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }
        if(auto expected_no_error = consume_character(tokenizer, 'l'); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        return null{};
    }

    [[nodiscard]] expected<value> consume_element(unicode::tokenizer& tokenizer){
        if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_next_character = tokenizer.peek_next();
        if(!expected_next_character){
            return unexpected{create_parse_error(tokenizer, expected_next_character.error())};
        }

        auto next_character = *expected_next_character;
        if(!next_character) return unexpected{create_parse_error(tokenizer, parse_error_reason::could_not_match_any_value_type)};

        if(auto expected_object = consume_object(tokenizer); expected_object && expected_object->has_value()){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return value(typename value::template in_place_index_t<json_type::object>{}, std::move(**expected_object));
        } else if (!expected_object) {
            return forward_error(std::move(expected_object));
        }
        if(auto expected_array = consume_array(tokenizer); expected_array && expected_array->has_value()){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return value{typename value::template in_place_index_t<json_type::array>{}, std::move(**expected_array)};
        } else if (!expected_array) {
            return forward_error(std::move(expected_array));
        }
        if(auto expected_string = consume_string(tokenizer); expected_string && *expected_string){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return value{typename value::template in_place_index_t<json_type::string>{}, std::move(**expected_string)};
        } else if (!expected_string){
          return forward_error(std::move(expected_string));
        }
        if(auto expected_boolean = consume_boolean(tokenizer); expected_boolean && *expected_boolean){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return value{typename value::template in_place_index_t<json_type::boolean>{}, std::move(**expected_boolean)};
        } else if (!expected_boolean){
            return forward_error(std::move(expected_boolean));
        }
        if(auto expected_null = consume_null(tokenizer); expected_null && *expected_null){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            return value{typename value::template in_place_index_t<json_type::null>{},basic_parser::null{}};
        } else if (!expected_null){
            return forward_error(std::move(expected_null));
        }
        if(auto expected_number = consume_number(tokenizer); expected_number && *expected_number){
            if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
                return forward_error(std::move(expected_no_error));
            }
            auto idx = (*expected_number)->index();
            if(idx == 0) return value{typename value::template in_place_index_t<json_type::integer>{}, std::move(std::get<0>(**expected_number))};
            if(idx == 1) return value{typename value::template in_place_index_t<json_type::floating_point>{}, std::move(std::get<1>(**expected_number))};
        } else if (!expected_number){
            return forward_error(std::move(expected_number));
        }

        return unexpected{create_parse_error(tokenizer, parse_error_reason::could_not_match_any_value_type)};
    }

    expected<std::optional<std::pair<string, value>>> consume_member(unicode::tokenizer& tokenizer){
        if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_member_key = consume_string(tokenizer);
        if(!expected_member_key){
            return forward_error(std::move(expected_member_key));
        }
        auto& member_key = *expected_member_key;
        if(!member_key) return {};

        if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_colon = tokenizer.peek_next();
        if(!expected_colon){
            return unexpected{create_parse_error(tokenizer, expected_colon.error())};
        }

        if(*expected_colon!= ':'){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::missing_colon_after_key)};
        }

        (void) tokenizer.consume_one();

        auto expected_element = consume_element(tokenizer);
        if(!expected_element && expected_element.error().reason == parse_error_reason::could_not_match_any_value_type){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::expected_element_after_key)};
        } else if (!expected_element){
            return forward_error(std::move(expected_element));
        }

        return std::pair<string, value>{std::move(*member_key), std::move(*expected_element)};
    }

    template <typename inserter>
    expected_no_value consume_members(unicode::tokenizer& tokenizer, inserter ins){
        auto expected_member = consume_member(tokenizer);
        if(!expected_member){
            return forward_error(std::move(expected_member));
        }

        (*ins).operator=(std::move(**expected_member));
        ++ins;
        while(true){
            auto expected_next = tokenizer.peek_next();
            if(!expected_next){
                return unexpected{create_parse_error(tokenizer, expected_next.error())};
            }
            if(*expected_next != ',') break;
            (void) tokenizer.consume_one();

            auto expected_member = consume_member(tokenizer);
            if(!expected_member){
                return forward_error(std::move(expected_member));
            }
            auto& member = *expected_member;
            if(!member) {
                (void) tokenizer.consume_one();
                return unexpected{create_parse_error(tokenizer, parse_error_reason::expected_brace)};
            }
            (*ins).operator=(std::move(*member));
        }

        return {};
    }

    expected<std::optional<object>> consume_object(unicode::tokenizer& tokenizer){
        auto expected_open_bracket = tokenizer.peek_next();
        if(!expected_open_bracket){
            return unexpected{create_parse_error(tokenizer, expected_open_bracket.error())};
        }

        if(*expected_open_bracket != '{') return std::nullopt;

        (void) tokenizer.consume_one();

        if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_closed_bracket = tokenizer.peek_next();
        if(!expected_closed_bracket){
            return unexpected{create_parse_error(tokenizer, expected_closed_bracket.error())};
        }

        if(*expected_closed_bracket == '}'){
            (void) tokenizer.consume_one();
            return object{get_allocator()};
        }

        object obj{get_allocator()};
        if(auto expected_no_error = consume_members(tokenizer, std::inserter(obj, obj.end())); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_again_close_bracket = tokenizer.peek_next();
        if(!expected_again_close_bracket){
            return unexpected{create_parse_error(tokenizer, expected_again_close_bracket.error())};
        }

        if(*expected_again_close_bracket != '}'){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::expected_closing_brace)};
        }
        (void) tokenizer.consume_one();
        return {std::move(obj)};
    }

    template <typename Iter>
    expected_no_value consume_elements(unicode::tokenizer& tokenizer, Iter ins){
        auto expected_element = consume_element(tokenizer);
        if(!expected_element){
            return forward_error(std::move(expected_element));
        }

        auto& element = *expected_element;

        (*ins).operator=(std::move(element));
        ++ins;
        while(true){
            auto expected_comma = tokenizer.peek_next();
            if(!expected_comma){
                return unexpected{create_parse_error(tokenizer, expected_comma.error())};
            }

            if(*expected_comma != ',') break;

            (void) tokenizer.consume_one();

            auto expected_element = consume_element(tokenizer);
            if(!expected_element) {
                return forward_error(std::move(expected_element));
            }

            (*ins).operator=(std::move(*expected_element));
        }

        return {};
    }

    expected<std::optional<array>> consume_array(unicode::tokenizer& tokenizer){
        auto expected_open_bracket = tokenizer.peek_next();
        if(!expected_open_bracket){
            return unexpected{create_parse_error(tokenizer, expected_open_bracket.error())};
        }

        if(*expected_open_bracket != '[') return std::nullopt;

        (void) tokenizer.consume_one();

        if(auto expected_no_error = consume_whitespace(tokenizer); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }
        auto expected_close_bracket = tokenizer.peek_next();
        if(!expected_close_bracket){
            return unexpected{create_parse_error(tokenizer, expected_close_bracket.error())};
        }

        if(*expected_close_bracket == ']') {
            (void) tokenizer.consume_one();
            return array{get_allocator()};
        }

        array arr{get_allocator()};

        if(auto expected_no_error = consume_elements(tokenizer, std::inserter(arr, arr.end())); !expected_no_error){
            return forward_error(std::move(expected_no_error));
        }

        auto expected_again_close_bracket = tokenizer.peek_next();
        if(!expected_again_close_bracket){
            return unexpected{create_parse_error(tokenizer, expected_again_close_bracket.error())};
        }

        if(*expected_again_close_bracket != ']'){
            (void) tokenizer.consume_one();
            return unexpected{create_parse_error(tokenizer, parse_error_reason::expected_closing_bracket)};
        }
        (void) tokenizer.consume_one();
        return {std::move(arr)};
    }
};

}