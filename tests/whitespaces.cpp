/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"

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

using parser = default_parser;

namespace{
    const std::string whitespaces = {' ', '\t', '\n', '\r', '\n', '\t', ' '};
}

TEST_CASE("Single element with whitespaces before"){
    parser parser_;
    auto result = parser_.parse(whitespaces + "null");

    REQUIRE(result);
    CHECK(result->index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with whitespaces after"){
    parser parser_;
    auto result = parser_.parse("null" + whitespaces);
    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with whitespaces before and after"){
    parser parser_;
    auto result = parser_.parse(whitespaces + "null" + whitespaces);

    REQUIRE(result.has_value());

    CHECK(result->index() == netformats::json::json_type::null);
}

TEST_CASE("Single element with no whitespaces"){
    parser parser_;
    auto result = parser_.parse( "null");

    REQUIRE(result);

    CHECK(result->index() == netformats::json::json_type::null);
}

TEST_CASE("Empty object with whitespaces"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().empty());
}

TEST_CASE("Empty object with no whitespaces"){
    parser parser_;
    auto result = parser_.parse("{" "}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().empty());
}

TEST_CASE("Empty array with whitespaces"){
    parser parser_;
    auto result = parser_.parse("[" + whitespaces + "]");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::array);
    CHECK(result->get<parser::array>().empty());
}

TEST_CASE("Empty array with no whitespaces"){
    parser parser_;
    auto result = parser_.parse("["  "]");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::array);
    CHECK(result->get<parser::array>().empty());
}

TEST_CASE("Object with whitespaces before and after key"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "\"property\"" + whitespaces + ": null}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with no whitespaces before and after key"){
    parser parser_;
    auto result = parser_.parse("{" "\"property\""  ": null}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with whitespaces before key"){
    parser parser_;
    auto result = parser_.parse("{" + whitespaces + "\"property\"" ": null}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Object with whitespaces after key"){
    parser parser_;
    auto result = parser_.parse("{" "\"property\"" + whitespaces + ": null}");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::object);
    CHECK(result->get<parser::object>().get_member("property").index() == netformats::json::json_type::null);
}

TEST_CASE("Whitespaces different than standard 0020"){
    parser parser_;
    std::string element = "{" "\"property\"" + std::string{"\u00A0"} + ": null}";
    auto result = parser_.parse(element);

    REQUIRE_FALSE(result.has_value());

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    REQUIRE(*result.error().buffer_iterator == *"\u00A0");
    REQUIRE(result.error().position == text_position{1,12});
    REQUIRE(result.error().reason == netformats::json::parse_error_reason::missing_colon_after_key);
}