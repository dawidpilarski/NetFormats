#include <netformats/basic_parser.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using parser = netformats::json::basic_parser<std::string, long long>;

TEST_CASE("Simple root with integer"){
    parser parser_;
    auto result = parser_.parse("1234");

    CHECK(result.index() == netformats::json::json_type::integer);
    CHECK(result.get<netformats::json::json_type::integer>() == 1234);
}

TEST_CASE("Simple root with string"){
    parser parser_;
    auto result = parser_.parse("\"1234\"");

    CHECK(result.index() == netformats::json::json_type::string);
    CHECK(result.get<netformats::json::json_type::string>() == "1234");
}

TEST_CASE("Simple root with object"){
    parser parser_;
    auto result = parser_.parse("{}");

    CHECK(result.index() == netformats::json::json_type::object);
    CHECK(result.get<netformats::json::json_type::object>() == parser::object{});
}

TEST_CASE("Simple root wtih array"){
    parser parser_;
    auto result = parser_.parse("[]");

    CHECK(result.index() == netformats::json::json_type::array);
    CHECK(result.get<netformats::json::json_type::array>() == parser::array{});
}

TEST_CASE("Simple root with floating point"){
    parser parser_;
    auto result = parser_.parse("123.456e10");

    CHECK(result.index() == netformats::json::json_type::floating_point);
    CHECK_THAT(result.get<netformats::json::json_type::floating_point>(), Catch::Matchers::WithinRel(123.456e10, 0.0001));
}

TEST_CASE("Simple root with null"){
    parser parser_;
    auto result = parser_.parse("null");

    CHECK(result.index() == netformats::json::json_type::null);
    CHECK(result.get<netformats::json::json_type::null>() == parser::null{});
}

TEST_CASE("Simple root with true"){
    parser parser_;
    auto result = parser_.parse("true");

    CHECK(result.index() == netformats::json::json_type::boolean);
    CHECK(result.get<netformats::json::json_type::boolean>() == true);
}

TEST_CASE("Simple root with false"){
    parser parser_;
    auto result = parser_.parse("false");

    CHECK(result.index() == netformats::json::json_type::boolean);
    CHECK(result.get<netformats::json::json_type::boolean>() == false);
}