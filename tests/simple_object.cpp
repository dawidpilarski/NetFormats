/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include <netformats/basic_parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using parser = netformats::json::basic_parser<std::string, long long>;

TEST_CASE("Simple object with string property"){
    parser parser_;
    auto result = parser_.parse(R"({"property": "value"})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::string>() == "value");
}

TEST_CASE("Simple object with 2 string properties"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": "value",
    "property2": "value2"
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::string>() == "value");
    CHECK(result.get<parser::object>().get_member("property2").get<parser::string>() == "value2");
}

TEST_CASE("Simple object with integer property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": 1234
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::integer>() == 1234);
}

TEST_CASE("Simple object with floating property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": 1234.567
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK_THAT(result.get<parser::object>().get_member("property").get<parser::floating_point>(), Catch::Matchers::WithinRel(1234.567, 0.0001));
}

TEST_CASE("Simple object with null property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": null
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::null>() == parser::null{});
}

TEST_CASE("Simple object with true property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": true
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::boolean>() == true);
}

TEST_CASE("Simple object with false property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": false
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::boolean>() == false);
}

TEST_CASE("Simple object with nested object with property"){
    parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
    }
})");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").get<parser::object>().get_member("nestedProperty").get<parser::null>() == parser::null{});
}