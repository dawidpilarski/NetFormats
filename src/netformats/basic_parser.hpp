/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once
#include <netformats/basic_value.hpp>
#include <netformats/basic_object.hpp>
#include <netformats/basic_array.hpp>
#include <netformats/unicode_tokenizer.hpp>
#include <charconv>

#include <array>
#include <system_error>
#include <charconv>
#include <algorithm>
#include <utility>
#include <stdexcept>

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

    template <typename Integer>
    Integer create_integer(const char* begin, const char* end);

    template <>
    inline long long create_integer<long long>(const char* begin, const char* end){
        long long integer;
        auto result = std::from_chars(begin, end, integer);
        if(result.ec == std::errc{}){
            return integer;
        } else[[unlikely]]{
            throw std::runtime_error("Could not parse integer");
        }
    }

template <typename string_,
          typename integer_,
          typename boolean_ = bool,
          typename floating_point_ = long double>
class basic_parser{
public:
    using value = basic_value<string_, integer_>;
    using object = typename value::object;
    using array = typename value::array;
    using string = typename value::string;
    using integer = typename value::integer;
    using boolean = typename value::boolean;
    using floating_point = typename value::floating_point;
    using null = typename value::null;

    //todo handle allocators correctly
    explicit basic_parser(std::allocator<int> alloc) : root_allocator(std::move(alloc)){}
    basic_parser() : basic_parser(std::allocator<int>{}){}


    [[nodiscard]] value parse(std::string_view json){
        unicode::tokenizer tokenizer(json);
        auto result = consume_element(tokenizer);
        if(!result){
            throw "Could not parse json. Empty?";
        }

        return *result;
    }

private:
    std::allocator<int> root_allocator;

    constexpr void consume_whitespace(unicode::tokenizer &tokenizer) {
        auto whitespace_characters = {u8'\u0020', u8'\u000A', u8'\u000D', u8'\u0009'};

        auto is_whitespace = [&whitespace_characters](char32_t character) {
            return std::any_of(whitespace_characters.begin(), whitespace_characters.end(), [&character](auto element) {
                return character == element;
            });
        };

        auto next_character = tokenizer.peek_next();
        if (!next_character) return;

        if (!is_whitespace(*next_character)) {
            return;
        }

        tokenizer.consume_one();
        while (is_whitespace(tokenizer.peek_next().value_or('A'))) {
            tokenizer.consume_one();
        }
    }

    enum class sign {
        plus = 1, minus = -1
    };

    constexpr sign consume_sign(unicode::tokenizer &tokenizer) {
        auto character = tokenizer.peek_next();
        if (!character.has_value()) return sign::plus;

        if (*character == '+') {
            tokenizer.consume_one();
            return sign::plus;
        }

        if (*character == '-') {
            tokenizer.consume_one();
            return sign::minus;
        }

        return sign::plus;
    }

    constexpr std::optional<char> consume_onenine(unicode::tokenizer &tokenizer) {
        auto character = tokenizer.peek_next();
        if (!character.has_value()) return {};

        if (*character >= '1' && *character <= '9') {
            tokenizer.consume_one();
            return static_cast<char>(*character);
        }

        return {};
    }

    constexpr std::optional<char> consume_digit(unicode::tokenizer &tokenizer) {
        auto character = tokenizer.peek_next();
        if (!character.has_value()) return {};

        if (*character == '0') {
            tokenizer.consume_one();
            return '0';
        }

        return consume_onenine(tokenizer);
    }

    constexpr std::optional<std::string_view> consume_digits(unicode::tokenizer &tokenizer) {
        std::optional<char> digit;

        const char *begin = nullptr;
        const char *end = nullptr;
        std::optional result = consume_digit(tokenizer);

        if (result) {
            begin = tokenizer.current_iterator();
            while (consume_digit(tokenizer));
            end = tokenizer.current_iterator() + 1;

            return std::string_view{begin, end};
        }

        return {};
    }

    std::optional<const char *> consume_integer(unicode::tokenizer &tokenizer) {
        signed multiplier = 1;
        const char *begin = nullptr;
        auto next_character = tokenizer.peek_next();
        if (!next_character) return {};
        if (*next_character == '-') {
            multiplier = -1;
            tokenizer.consume_one();
            begin = tokenizer.current_iterator();
        }

        if (tokenizer.peek_next() == '0') {
            tokenizer.consume_one();
            if (!begin) begin = tokenizer.current_iterator();
            if (consume_digit(tokenizer)) {
                throw std::runtime_error("Parsing json failed at " + tokenizer.source_position() + "."
                                                                                                   "Integer cannot have 2 leading zeros.");
            }

            return begin;
        }

        auto firstOneNine = consume_onenine(tokenizer);
        if (!firstOneNine && multiplier == -1) {
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() +
                                     ". Integer cannot consist only of \"-\".");
        }
        if (!firstOneNine) {
            return {};
        }

        if (!begin) begin = tokenizer.current_iterator();

        consume_digits(tokenizer);

        return begin;
    }

    std::optional<std::string_view> consume_fraction(unicode::tokenizer &tokenizer) {
        if (tokenizer.peek_next() != '.') return {};

        tokenizer.consume_one();
        const char *begin = tokenizer.current_iterator();

        const auto digits = consume_digits(tokenizer);
        if (!digits)
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position()
                                     + ". Expected number fraction after \".\".");

        const auto &parsed_digits = *digits;

        return std::string_view{begin, parsed_digits.data() + parsed_digits.size()};
    }

    std::optional<std::string_view> consume_exponent(unicode::tokenizer &tokenizer) {
        auto next = tokenizer.peek_next();
        if (!next) return {};
        if (*next != 'e' && *next != 'E') return {};

        tokenizer.consume_one();
        const char *begin = tokenizer.current_iterator();

        consume_sign(tokenizer);
        const auto consumed_digits = consume_digits(tokenizer);
        if (!consumed_digits)
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position()
                                     + R"(. Expected digits or "+"/"-" sign after "e")");

        return std::string_view{begin, consumed_digits->data() + consumed_digits->size()};
    }

    std::optional<std::variant<integer, floating_point>> consume_number(unicode::tokenizer &tokenizer) {
        const auto consumed_integer = consume_integer(tokenizer);
        if (!consumed_integer) return {};

        const auto consumed_fraction = consume_fraction(tokenizer);
        const auto consumed_exponent = consume_exponent(tokenizer);

        if (!consumed_fraction && !consumed_exponent) {
            return create_integer<integer>(*consumed_integer, tokenizer.current_iterator() + 1);
        }

        long double number;
        const char *begin = *consumed_integer;
        auto result = std::from_chars(begin, tokenizer.current_iterator()+1, number);

        if (result.ec != std::errc{}) {
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". "
                                                                                                "Could not interpret integers as double");
        }

        return number;
    }

    std::optional<char> consume_hex(unicode::tokenizer &tokenizer) {
        auto next_character = tokenizer.peek_next();
        if (!next_character) return {};
        if ((*next_character >= 'A' && *next_character <= 'F') ||
            *next_character >= 'a' && *next_character <= 'f') {
            tokenizer.consume_one();
            return static_cast<char>(*next_character);
        }

        if (std::optional<char> digit; (digit = consume_digit(tokenizer))) {
            return *digit;
        }

        return {};
    }

    std::optional<char32_t> consume_escaped(unicode::tokenizer &tokenizer) {
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

        auto next_character = tokenizer.peek_next();
        if (!next_character) return {};
        if (std::any_of(escaped_characters.begin(), escaped_characters.end(), [&next_character](char character) {
            return *next_character == character;
        })) {
            tokenizer.consume_one();
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

        tokenizer.consume_one();
        auto first_hex_iter = tokenizer.current_iterator();
        std::array four_hexes = {
                consume_hex(tokenizer),
                consume_hex(tokenizer),
                consume_hex(tokenizer),
                consume_hex(tokenizer)};

        if (std::any_of(std::begin(four_hexes), std::end(four_hexes), [](const auto &element) {
            return !element.has_value();
        })) {
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() +
                                     ". Invalid hex character");
        }

        ++first_hex_iter;

        uint32_t character_from_hex;
        std::from_chars(first_hex_iter, tokenizer.current_iterator() + 1, character_from_hex, 16);
        return std::bit_cast<char32_t>(character_from_hex);
    }

    std::optional<char32_t> consume_character(unicode::tokenizer &tokenizer) {
        auto next_character = tokenizer.peek_next();
        if (!next_character) return {};

        if (*next_character == '\"') return {};
        if (*next_character == '\\') {
            tokenizer.consume_one();
            auto consumed_character = consume_escaped(tokenizer);
            //todo real character should be returned instead of escape + character
            if (!consumed_character)
                throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() +
                                         ".\n"
                                         "Expected escaped character after \\:\n"
                                         "[\", \\, /, b, f, n,  r, t, u(hex, hex, hex, hex)]");
            return *consumed_character;
        }

        tokenizer.consume_one();
        return *next_character;
    }

    std::optional<std::string> consume_characters(unicode::tokenizer &tokenizer) {
        std::string characters;
        std::optional<char32_t> character;

        while ((character = consume_character(tokenizer))) {
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

    std::optional<std::string> consume_string(unicode::tokenizer &tokenizer) {
        std::optional<char32_t> next_character = tokenizer.peek_next();
        if (next_character != '\"') return {};

        tokenizer.consume_one();
        auto result = consume_characters(tokenizer);

        next_character = tokenizer.peek_next();
        if (next_character != '\"')
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                     "String does not end with '\"'");

        tokenizer.consume_one();

        if (result.has_value()) return *result;
        return std::string{""};
    }

    void consume_character(unicode::tokenizer& tokenizer, char character){
        auto next_character = tokenizer.peek_next();
        if(next_character != character) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                                                 "Expected \"" + character + "\"");

        tokenizer.consume_one();
    }

    std::optional<bool> consume_boolean(unicode::tokenizer& tokenizer){
        auto next_character = tokenizer.peek_next();
        if(!next_character) return {};

        if(*next_character == 't'){
            tokenizer.consume_one();
            consume_character(tokenizer, 'r');
            consume_character(tokenizer, 'u');
            consume_character(tokenizer, 'e');
            return true;
        } else if(*next_character == 'f'){
            tokenizer.consume_one();
            consume_character(tokenizer, 'a');
            consume_character(tokenizer, 'l');
            consume_character(tokenizer, 's');
            consume_character(tokenizer, 'e');
            return false;
        }
        return {};
    }

    bool consume_null(unicode::tokenizer& tokenizer){
        auto next_character = tokenizer.peek_next();
        if(next_character != 'n') return false;
        tokenizer.consume_one();
        consume_character(tokenizer, 'u');
        consume_character(tokenizer, 'l');
        consume_character(tokenizer, 'l');

        return true;
    }

    std::optional<basic_value<string, integer>> consume_element(unicode::tokenizer& tokenizer){
        consume_whitespace(tokenizer);

        auto next_character = tokenizer.peek_next();
        if(!next_character) return {};

        if(auto object = consume_object(tokenizer); object){
            consume_whitespace(tokenizer);
            return value(*object);
        }
        if(auto array = consume_array(tokenizer); array){
            consume_whitespace(tokenizer);
            return value{*array};
        }
        if(auto string = consume_string(tokenizer); string){
            consume_whitespace(tokenizer);
            return value{*string};
        }
        if(auto boolean = consume_boolean(tokenizer); boolean){
            consume_whitespace(tokenizer);
            return value{*boolean};
        }
        if(auto null = consume_null(tokenizer); null){
            consume_whitespace(tokenizer);
            return value{basic_parser::null{}};
        }
        if(auto number = consume_number(tokenizer); number){
            consume_whitespace(tokenizer);
            auto idx  = number->index();
            if(idx == 0) return value{std::get<integer>(*number)};
            if(idx == 1) return value{std::get<floating_point>(*number)};
        }

        throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                 "Expected element, but could not parse it.'\"'");
    }

    std::optional<std::pair<string, value>> consume_member(unicode::tokenizer& tokenizer){
        consume_whitespace(tokenizer);

        std::optional member_key = consume_string(tokenizer);
        if(!member_key) return {};

        consume_whitespace(tokenizer);
        if(tokenizer.peek_next() != ':') throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n"
                                                                                                                             "Expected \":\" after string in member definition");
        tokenizer.consume_one();
        std::optional element = consume_element(tokenizer);
        if(!element) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n"
                                                                                                         "Expected element value after \":\"");

        return std::pair<std::string, basic_value<std::string, long long>>{*member_key, *element};
    }

    template <typename inserter>
    void consume_members(unicode::tokenizer& tokenizer, inserter ins){
        auto member = consume_member(tokenizer);
        if(!member) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                             "Could not parse member");

        *ins = *member;
        ++ins;
        while((tokenizer.peek_next() == ',')){
            tokenizer.consume_one();
            member = consume_member(tokenizer);
            if(!member) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                                 "Expected member after \",\"");
            *ins = *member;
        }
    }

    std::optional<object> consume_object(unicode::tokenizer& tokenizer){
        if(tokenizer.peek_next() != '{') return std::nullopt;

        tokenizer.consume_one();
        consume_whitespace(tokenizer);
        if(tokenizer.peek_next() == '}'){
            tokenizer.consume_one();
            return object{};
        }

        object obj;
        consume_members(tokenizer, std::inserter(obj, obj.end()));
        return obj;
    }

    template <typename Iter>
    void consume_elements(unicode::tokenizer& tokenizer, Iter  ins){
        auto element = consume_element(tokenizer);
        if(!element) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                              "Could not parse member");

        *ins = *element;
        ++ins;
        while(tokenizer.peek_next() == ','){
            tokenizer.consume_one();
            element = consume_element(tokenizer);
            if(!element) throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                                        "Expected element after \",\"");
            *ins = *element;
        }
    }

    std::optional<array> consume_array(unicode::tokenizer& tokenizer){
        if(tokenizer.peek_next() != '[') return std::nullopt;

        tokenizer.consume_one();
        consume_whitespace(tokenizer);
        if(tokenizer.peek_next() == ']') return array{};

        array arr{};

        consume_elements(tokenizer, std::inserter(arr, arr.end()));
        if(tokenizer.peek_next() != ']'){
            throw std::runtime_error("Parsing json failed at: " + tokenizer.source_position() + ". \n" +
                                     "Expected end of array \"]\"");
        }
        return arr;
    }
};

}