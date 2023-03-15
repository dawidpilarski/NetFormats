/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"

using parser = default_parser;

TEST_CASE("Simple integer"){
    parser parser_;
    auto result = parser_.parse("1234");

    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::integer);
    CHECK(result->get<parser::integer>() == 1234);
}

TEST_CASE("Integer with letter instead inside in root"){
    parser parser_;
    auto result = parser_.parse("12f4");

    REQUIRE_FALSE(result);

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    CHECK(*result.error().buffer_iterator == 'f');
    CHECK(result.error().position == text_position{1,3});
    CHECK(result.error().reason == netformats::json::parse_error_reason::remaining_data_after_json_parse);
}

TEST_CASE("Integer with letter in property"){
    parser parser_;
    auto result = parser_.parse(R"({"property": 12f4})");

    REQUIRE_FALSE(result);

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    CHECK(*result.error().buffer_iterator == 'f');
    CAPTURE(result.error().position.column);
    CHECK(result.error().position == text_position{1,16});
    CAPTURE(result.error().reason);
    CHECK(result.error().reason == netformats::json::parse_error_reason::expected_closing_brace);
}

TEST_CASE("Negative integer"){

    parser parser_;
    auto result = parser_.parse("-1234");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::integer);
    CHECK(result->get<parser::integer>() == -1234);
}

TEST_CASE("zero"){

    parser parser_;
    auto result = parser_.parse("0");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::integer);
    CHECK(result->get<parser::integer>() == 0);
}

TEST_CASE("minus zero"){

    parser parser_;
    auto result = parser_.parse("-0");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::integer);
    CHECK(result->get<parser::integer>() == -0);
}

TEST_CASE("Single digit"){

    parser parser_;

    for(long int digit=0; digit < 10; ++digit){
        auto result = parser_.parse(std::to_string(digit));
        REQUIRE(result);

        REQUIRE(result->index() == netformats::json::json_type::integer);
        CAPTURE(result->get<parser::integer>());
        static_assert(parser::value::can_store_v<parser::integer>);
        CHECK(result->get<parser::integer>() == digit);

        auto result2 = parser_.parse("-" + std::to_string(digit));
        REQUIRE(result2->index() == netformats::json::json_type::integer);
        CAPTURE(result2->get<parser::integer>());
        CHECK(result2->get<parser::integer>() == -digit);
    }
}

TEST_CASE("integer with mutiple digits starting from 0"){
    parser parser_;

    auto result = parser_.parse("019");
    REQUIRE_FALSE(result);

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    CHECK(*result.error().buffer_iterator == '1');
    CHECK(result.error().position == text_position{1,2});
    CHECK(result.error().reason == netformats::json::parse_error_reason::integer_0_with_multiple_digits);
}

TEST_CASE("Simple floating point "){
    parser parser_;

    auto result = parser_.parse("123.456");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(123.456, 0.001));
}

TEST_CASE("Floating point zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.0");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(123.0, 0.001));
}

TEST_CASE("Floating point zero zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.00");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(123.0, 0.001));
}

TEST_CASE("Floating point one zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.10");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(123.10, 0.001));
}

TEST_CASE("Floating point no fraction with exponent"){
    parser parser_;

    auto result = parser_.parse("123e2");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(12300, 0.00001));
}

TEST_CASE("Floating point with exponent"){
    parser parser_;

    auto result = parser_.parse("123.1e2");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with capital E exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E2");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with explicit + exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E+2");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with negative exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E-2");
    REQUIRE(result);

    REQUIRE(result->index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result->get<parser::floating_point>(), Catch::Matchers::WithinRel(1.2310, 0.0001));
}

TEST_CASE("Floating point with comma instead of dot"){
    parser parser_;

    auto result = parser_.parse("123,1E-2");
    REQUIRE_FALSE(result);

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    CHECK(*result.error().buffer_iterator == ',');
    CHECK(result.error().position == text_position{1, 4});
    CHECK(result.error().reason == netformats::json::parse_error_reason::remaining_data_after_json_parse);
}

TEST_CASE("Floating point with comma instead of dot in the object"){
    parser parser_;

    auto result = parser_.parse(R"({"value": 123,1E-2})");
    REQUIRE_FALSE(result);

    REQUIRE(result.error().buffer_iterator != result.error().buffer.end());
    CHECK(*result.error().buffer_iterator != ',');
    CHECK(result.error().position != text_position{1, 14});
    CHECK(result.error().reason == netformats::json::parse_error_reason::expected_brace);
}