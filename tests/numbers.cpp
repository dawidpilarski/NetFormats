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

using parser = netformats::json::basic_parser<std::string, long long>;

TEST_CASE("Simple integer"){

    parser parser_;
    auto result = parser_.parse("1234");

    REQUIRE(result.index() == netformats::json::json_type::integer);
    CHECK(result.get<parser::integer>() == 1234);
}

TEST_CASE("Negative integer"){

    parser parser_;
    auto result = parser_.parse("-1234");

    REQUIRE(result.index() == netformats::json::json_type::integer);
    CHECK(result.get<parser::integer>() == -1234);
}

TEST_CASE("zero"){

    parser parser_;
    auto result = parser_.parse("0");

    REQUIRE(result.index() == netformats::json::json_type::integer);
    CHECK(result.get<parser::integer>() == 0);
}

TEST_CASE("minus zero"){

    parser parser_;
    auto result = parser_.parse("-0");

    REQUIRE(result.index() == netformats::json::json_type::integer);
    CHECK(result.get<parser::integer>() == -0);
}

TEST_CASE("Single digit"){

    parser parser_;

    for(long int digit=0; digit < 10; ++digit){
        auto result = parser_.parse(std::to_string(digit));

        REQUIRE(result.index() == netformats::json::json_type::integer);
        CAPTURE(result.get<parser::integer>());
        CHECK(result.get<parser::integer>() == digit);

        result = parser_.parse("-" + std::to_string(digit));
        REQUIRE(result.index() == netformats::json::json_type::integer);
        CAPTURE(result.get<parser::integer>());
        CHECK(result.get<parser::integer>() == -digit);
    }
}

//todo negative test 019

TEST_CASE("Simple floating point "){
    parser parser_;

    auto result = parser_.parse("123.456");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(123.456, 0.001));
}

TEST_CASE("Floating point zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.0");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(123.0, 0.001));
}

TEST_CASE("Floating point zero zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.00");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(123.0, 0.001));
}

TEST_CASE("Floating point one zero fraction"){
    parser parser_;

    auto result = parser_.parse("123.10");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(123.10, 0.001));
}

TEST_CASE("Floating point no fraction with exponent"){
    parser parser_;

    auto result = parser_.parse("123e2");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(12300, 0.00001));
}

TEST_CASE("Floating point with exponent"){
    parser parser_;

    auto result = parser_.parse("123.1e2");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with capital E exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E2");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with explicit + exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E+2");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(12310, 0.00001));
}

TEST_CASE("Floating point with negative exponent"){
    parser parser_;

    auto result = parser_.parse("123.1E-2");

    REQUIRE(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<parser::floating_point>(), Catch::Matchers::WithinRel(1.2310, 0.0001));
}

//todo add negative tests