/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#include "test_defs.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>

#include <iostream>

TEST_CASE("Unfinished strings"){
    default_parser parser_;
    std::string string = {'\"', 'a', 'b', 'c', 'd'};
    auto result = parser_.parse(string);
    REQUIRE(!result.has_value());

    auto error = result.error();
    auto display_error = to_string(error);

    CHECK(display_error == R"(Parsing failed at position [line:column] 1:5
Reason: Invalid string. When parsing string, ending '"' character was not found.

"abcd
  ~~~^~~~)");
}

TEST_CASE("Object with missing ending brace. Last line starts with space"){
    default_parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty": null
})");
    REQUIRE_FALSE(result);

    auto error = result.error();
    auto display_error = to_string(error);

    CHECK(display_error == R"(Parsing failed at position [line:column] 4:1
Reason: Invalid object. After parsing objects members, ending '}' character was not found.

}
~^~~~)");
}


TEST_CASE("Object nested properties and missing ,."){
    default_parser parser_;
    auto result = parser_.parse(R"({
    "property": {
       "nestedProperty1": null
       "nestedProperty2": null
    }
})");
    REQUIRE_FALSE(result);

    auto error = result.error();
    auto display_error = to_string(error);

    // clion sometimes tends to remove trailing spaces, so we add it here in such a way it cannot remove it
    CHECK_THAT(display_error, Catch::Matchers::Equals(R"(Parsing failed at position [line:column] 4:8
Reason: Invalid object. After parsing objects members, ending '}' character was not found.

       "nestedProperty2":)" " " R"(
    ~~~^~~~)"));
}