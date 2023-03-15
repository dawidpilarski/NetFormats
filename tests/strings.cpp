/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"

using parser = default_parser;

TEST_CASE("Simple string"){
    parser parser_;
    auto result = parser_.parse("\"test\"");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "test");
}

TEST_CASE("Empty string"){
    parser parser_;
    auto result = parser_.parse("\"\"");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "");
}

TEST_CASE("String with escaped characters"){
    parser parser_;
    auto result = parser_.parse(R"("\"\\\/\b\f\n\r\t")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "\"\\/\b\f\n\r\t");
}

TEST_CASE("String with escaped zero hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u0000")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == std::string{'\0'});
}

TEST_CASE("String with 0null0 hex character"){
    parser parser_;
    auto result = parser_.parse(R"("0\u00000")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    std::string expectedString = {'0', '\0', '0'};

    auto string = (*result).get<netformats::json::json_type::string>();

    CHECK(string[0] == '0');
    CHECK(string[1] == '\0');
    CHECK(string[2] == '0');
}

TEST_CASE("String with escaped hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u1234")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "\U00001234");
}

TEST_CASE("String with another escaped hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u0019")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "\U00000019");
}

TEST_CASE("String Zażółć gęślą jaźń"){
    parser parser_;
    auto result = parser_.parse(R"("Zażółć gęślą jaźń")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "Zażółć gęślą jaźń");
}

TEST_CASE("String spanish"){
    parser parser_;
    auto result = parser_.parse(R"("El veloz murciélago hindú comía feliz cardillo y kiwi. La cigüeña tocaba el saxofón detrás del palenque de paja.")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "El veloz murciélago hindú comía feliz cardillo y kiwi. La cigüeña tocaba el saxofón detrás del palenque de paja.");
}


TEST_CASE("String cyryllic"){
    parser parser_;
    auto result = parser_.parse(R"("Съешь же ещё этих мягких французских")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "Съешь же ещё этих мягких французских");
}

TEST_CASE("String 世丕且且世两上与丑万丣丕且丗丕"){
    parser parser_;
    std::string string = "世丕且且世两上与丑万丣丕且丗丕";
    auto result = parser_.parse(R"("世丕且且世两上与丑万丣丕且丗丕")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "世丕且且世两上与丑万丣丕且丗丕");
}

TEST_CASE("String hex with capital U"){
    parser parser_;
    std::string string = "\"\\U123\"";
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::escaped_character_invalid);\
    REQUIRE(error.buffer_iterator < error.buffer.end());
    CHECK(*error.buffer_iterator == 'U');
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,3});
}

TEST_CASE("String hex with not enough hex numbers"){
    parser parser_;
    std::string string = "\"\\uabc\"";
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::hex_invalid);
    REQUIRE(error.buffer_iterator < error.buffer.end());
    CHECK(*error.buffer_iterator == '\"');
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,7});
}

TEST_CASE("String hex with 0 hex numbers"){
    parser parser_;
    std::string string = "\"\\u\"";
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::hex_invalid);
    REQUIRE(error.buffer_iterator < error.buffer.end());
    CHECK(*error.buffer_iterator == '\"');
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,4});
}

TEST_CASE("String hex with letter g"){
    parser parser_;
    std::string string = "\"\\ufffg\"";
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::hex_invalid);
    REQUIRE(error.buffer_iterator < error.buffer.end());
    CHECK(*error.buffer_iterator == 'g');
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,7});
}

TEST_CASE("String with character int(19)"){
    parser parser_;
    std::string string = {'\"', 'a', 'b', 'c', char{19}, '\"'};
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::utf_8_codepoint_out_of_range);
    REQUIRE(error.buffer_iterator < error.buffer.end());
    CHECK(*error.buffer_iterator == char{19});
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,4});
}

TEST_CASE("String without closing quote"){
    parser parser_;
    std::string string = {'\"', 'a', 'b', 'c', 'd'};
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::string_missing_finishing_quote);
    REQUIRE(error.buffer_iterator == error.buffer.end());
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,5});
}

TEST_CASE("String with badly encoded utf-8"){
    parser parser_;
    std::string string = {'\"', char(0b11001111), char(0xff), '\"'};
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();

    CHECK(error.reason == netformats::json::parse_error_reason::invalid_utf_8_encoding);
    REQUIRE(error.buffer_iterator != error.buffer.end());
    CHECK(*error.buffer_iterator == char(0b11001111));
    CAPTURE(error.position.column);
    CHECK(error.position == text_position{1,1});
}