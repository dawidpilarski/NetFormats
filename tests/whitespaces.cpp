/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include <netformats/basic_parser.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

/*
 * whitespaces are ' ' 0x0020, '\r' 0x00A, '\n' 0x00D, '\t '0x009' or nothing.
 * No other whitespaces should be accepted (see JsonMcKeeman.txt)
 *
 * whitespaces occur in:
 * - member (before and after key),
 * - the element (before and after value),
 * - empty object
 * - empty array
 */

using parser = netformats::json::basic_parser<std::string, long long>;

namespace{
    const std::string whitespaces = {' ', '\t', '\n', '\r', '\n', '\t', ' '};
}

TEST_CASE("Single element with whitespaces before"){
    parser parser_;
    auto result = parser_.parse(whitespaces + "null");

    CHECK(result.index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with whitespaces after"){
    parser parser_;
    auto result = parser_.parse("null" + whitespaces);

    CHECK(result.index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with whitespaces before and after"){
    parser parser_;
    auto result = parser_.parse(whitespaces + "null" + whitespaces);

    CHECK(result.index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with no whitespaces"){
    parser parser_;
    auto result = parser_.parse( "null");

    CHECK(result.index() == netformats::json::json_type::null);
}

TEST_CASE("Empty object with whitespaces"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().empty());
}

TEST_CASE("Empty object with no whitespaces"){
    parser parser_;
    auto result = parser_.parse("{" "}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().empty());
}

TEST_CASE("Empty array with whitespaces"){
    parser parser_;
    auto result = parser_.parse("[" + whitespaces + "]");

    REQUIRE(result.index() == netformats::json::json_type::array);
    CHECK(result.get<parser::array>().empty());
}

TEST_CASE("Empty array with no whitespaces"){
    parser parser_;
    auto result = parser_.parse("["  "]");

    REQUIRE(result.index() == netformats::json::json_type::array);
    CHECK(result.get<parser::array>().empty());
}

TEST_CASE("Object with whitespaces before and after key"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "\"property\"" + whitespaces + ": null}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with no whitespaces before and after key"){
    parser parser_;
    auto result = parser_.parse("{" "\"property\""  ": null}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with whitespaces before key"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "\"property\"" ": null}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with whitespaces after key"){
    parser parser_;
    auto result = parser_.parse("{" "\"property\"" + whitespaces + ": null}");

    REQUIRE(result.index() == netformats::json::json_type::object);
    CHECK(result.get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

// Todo: test for whitespace before ',' after member

// Todo: add utf-8 unsupported whitespaces tests:
// https://en.wikipedia.org/wiki/Whitespace_character