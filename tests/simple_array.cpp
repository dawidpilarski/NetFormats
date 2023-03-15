/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"

using parser = default_parser;

TEST_CASE("Simple array of integers"){
    parser parser_;
    auto val = parser_.parse("[1, 2, 3]");
    REQUIRE(val.has_value());

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "integer");
    }
    CHECK(arr[0].get<parser::integer>() == 1);
    CHECK(arr[1].get<parser::integer>() == 2);
    CHECK(arr[2].get<parser::integer>() == 3);
}

TEST_CASE("Simple array of strings"){
    parser parser_;
    auto val = parser_.parse(R"(["1", "2", "3"])");
    REQUIRE(val);

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "string");
        CHECK(val.index() == netformats::json::json_type::string);
    }
    CHECK(arr[0].get<parser::string>() == "1");
    CHECK(arr[1].get<parser::string>() == "2");
    CHECK(arr[2].get<parser::string>() == "3");
}

TEST_CASE("Simple array of floating point"){
    parser parser_;
    auto val = parser_.parse(R"([1.01, 2.02, 3.03])");

    REQUIRE(val);

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        REQUIRE(to_string(val.index()) == "floating point");
    }

    CHECK_THAT(arr[0].get<parser::floating_point>(), Catch::Matchers::WithinRel(1.01, 0.00001));
    CHECK_THAT(arr[1].get<parser::floating_point>(), Catch::Matchers::WithinRel(2.02, 0.00001));
    CHECK_THAT(arr[2].get<parser::floating_point>(), Catch::Matchers::WithinRel(3.03, 0.00001));
}

TEST_CASE("Simple array of objects"){
    parser parser_;
    auto val = parser_.parse(R"([{}, {}, {}])");

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "object");
        CHECK(val.get<parser::object>() == parser::object{});
    }
}

TEST_CASE("Simple array of arrays"){
    parser parser_;
    auto val = parser_.parse(R"([[], [], []])");

    if(val){

    } else {
        REQUIRE(false);
    }

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "array");
        CHECK(val.get<parser::array>() == parser::array{});
    }
}

TEST_CASE("Simple array of trues"){
    parser parser_;
    auto val = parser_.parse(R"([true, true, true])");

    REQUIRE(val);

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "boolean");
        CHECK(val.get<parser::boolean>() == true);
    }
}

TEST_CASE("Simple array of falses"){
    parser parser_;
    auto val = parser_.parse(R"([false, false, false])");

    REQUIRE(val.has_value());

    REQUIRE(!val->holds_alternative<parser::object>());
    REQUIRE(val->holds_alternative<parser::array>());

    parser::array arr= val->get<parser::array>();
    for(const auto& val : arr){
        CHECK(to_string(val.index()) == "boolean");
        CHECK(val.get<parser::boolean>() == false);
    }
}

TEST_CASE("Array with missing comma"){
    parser parser_;
    auto val = parser_.parse(R"([false false, false])");

    REQUIRE_FALSE(val.has_value());

    REQUIRE(val.error().buffer_iterator != val.error().buffer.end());
    CHECK(*val.error().buffer_iterator == 'f');
    CHECK(val.error().position == text_position{1, 8});
    CHECK(val.error().reason == netformats::json::parse_error_reason::expected_closing_bracket);

}

TEST_CASE("Array with missing closing bracket"){
    parser parser_;
    auto val = parser_.parse(R"([false, false, false)");

    REQUIRE_FALSE(val.has_value());

    REQUIRE(val.error().buffer_iterator == val.error().buffer.end());
    CHECK(val.error().position == text_position{1, 20});
    CHECK(val.error().reason == netformats::json::parse_error_reason::expected_closing_bracket);
}

TEST_CASE("Array with missing closing bracket and newline"){
    parser parser_;
    auto val = parser_.parse(R"([false, false
 )");


    REQUIRE_FALSE(val.has_value());

    REQUIRE(val.error().buffer_iterator == val.error().buffer.end());
    CAPTURE(val.error().position.column);
    CHECK(val.error().position == text_position{2, 1});
    CHECK(val.error().reason == netformats::json::parse_error_reason::expected_closing_bracket);
}
