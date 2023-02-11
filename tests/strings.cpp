/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include <netformats/basic_value.hpp>
#include <netformats/basic_object.hpp>
#include <netformats/basic_array.hpp>
#include <netformats/basic_parser.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <iostream>

using parser = netformats::json::basic_parser<std::string, long long>;

TEST_CASE("Simple string"){
    parser parser_;
    auto result = parser_.parse("\"test\"");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "test");
}

TEST_CASE("Empty string"){
    parser parser_;
    auto result = parser_.parse("\"\"");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "");
}

TEST_CASE("String with escaped characters"){
    parser parser_;
    auto result = parser_.parse(R"("\"\\\/\b\f\n\r\t")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "\"\\/\b\f\n\r\t");
}

TEST_CASE("String with escaped zero hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u0000")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == std::string{'\0'});
}

TEST_CASE("String with 0\\u00000 hex character"){
    parser parser_;
    auto result = parser_.parse(R"("0\u00000")");

    CHECK(result.index() == netformats::json::json_type::string);
    std::string expectedString = {'0', '\0', '0'};

    CHECK(result.get<netformats::json::json_type::string>() == expectedString);
}

TEST_CASE("String with escaped hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u1234")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "\x12\x34");
}

TEST_CASE("String with another escaped hex character"){
    parser parser_;
    auto result = parser_.parse(R"("\u0019")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "\x19");
}

TEST_CASE("String Zażółć gęślą jaźń"){
    parser parser_;
    auto result = parser_.parse(R"("Zażółć gęślą jaźń")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "Zażółć gęślą jaźń");
}

TEST_CASE("String spanish"){
    parser parser_;
    auto result = parser_.parse(R"("El veloz murciélago hindú comía feliz cardillo y kiwi. La cigüeña tocaba el saxofón detrás del palenque de paja.")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "El veloz murciélago hindú comía feliz cardillo y kiwi. La cigüeña tocaba el saxofón detrás del palenque de paja.");
}


TEST_CASE("String cyryllic"){
    parser parser_;
    auto result = parser_.parse(R"("Съешь же ещё этих мягких французских")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "Съешь же ещё этих мягких французских");
}

TEST_CASE("String 世丕且且世两上与丑万丣丕且丗丕"){
    parser parser_;
    std::string string = "世丕且且世两上与丑万丣丕且丗丕";
    auto result = parser_.parse(R"("世丕且且世两上与丑万丣丕且丗丕")");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "世丕且且世两上与丑万丣丕且丗丕");
}


//todo negative test hex with capital U
//todo negative test hex of 3-0 characters
//todo negative test hex of letters GHIJ in hex
//todo negative test hex of letters GHIJ in hex
//todo negative test character < 20
//todo negative test character > 10ffff