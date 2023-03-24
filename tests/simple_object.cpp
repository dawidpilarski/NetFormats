/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"

using parser = default_parser;

TEST_CASE("Simple object with string property"){
    parser parser_;
    auto result = parser_.parse(R"({"property": "value"})");
    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::string>() == "value");
}

TEST_CASE("Simple object with 2 string properties"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": "value",
    "property2": "value2"
})");

    REQUIRE(result.has_value());

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::string>() == "value");
    CHECK(result->get<parser::object>().get_member("property2").get<parser::string>() == "value2");
}

TEST_CASE("Simple object with integer property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": 1234
})");

    REQUIRE(result.has_value());

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::integer>() == 1234);
}

TEST_CASE("Simple object with floating property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": 1234.567
})");

    REQUIRE(result.has_value());

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK_THAT(result->get<parser::object>().get_member("property").get<parser::floating_point>(), Catch::Matchers::WithinRel(1234.567, 0.0001));
}

TEST_CASE("Simple object with null property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": null
})");

    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::null>() == parser::null{});
}

TEST_CASE("Simple object with true property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": true
})");

    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::boolean>() == true);
}

TEST_CASE("Simple object with false property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": false
})");

    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").get<parser::boolean>() == false);
}

TEST_CASE("Simple object with nested object with property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
    }
})");

    REQUIRE(result.has_value());

    CHECK((*result).index() == netformats::json::json_type::object);
    CHECK((*result).get<parser::object>().get_member("property").get<parser::object>().get_member("nestedProperty").get<parser::null>() == parser::null{});
}

TEST_CASE("Empty object with missing ending brace"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
    }
)");
    REQUIRE_FALSE(result);

    CHECK(result.error().buffer_iterator == result.error().buffer.end());
    CHECK(result.error().position == text_position{5, 0});
    CHECK(result.error().reason == netformats::json::parse_error_reason::expected_closing_brace);
}

TEST_CASE("Empty object with missing ending brace. Last line starts with space"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
    }
 )");
    REQUIRE_FALSE(result);

    CHECK(result.error().buffer_iterator == result.error().buffer.end());
    CHECK(result.error().position == text_position{5, 1});
    CHECK(result.error().reason == netformats::json::parse_error_reason::expected_closing_brace);
}

TEST_CASE("Nested object with missing ending brace. Last line starts with space"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
})");
    REQUIRE_FALSE(result);

    CHECK(result.error().buffer_iterator == result.error().buffer.end());
    CHECK(result.error().position == text_position{4, 1});
    CHECK(result.error().reason == netformats::json::parse_error_reason::expected_closing_brace);
}