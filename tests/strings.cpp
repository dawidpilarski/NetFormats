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
    CHECK((*result).get<netformats::json::json_type::string>() == "\x12\x34");
}

TEST_CASE("String with another escaped hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u0019")");
    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::string);
    CHECK((*result).get<netformats::json::json_type::string>() == "\x19");
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


//todo negative test hex with capital U
//todo negative test hex of 3-0 characters
//todo negative test hex of letters GHIJ in hex
//todo negative test hex of letters GHIJ in hex
//todo negative test character < 20
//todo negative test character > 10ffff