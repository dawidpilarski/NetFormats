/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/null.hpp>
#include <netformats/type_traits.hpp>

#include <string>
#include <scoped_allocator>

namespace netformats::json{

    template <template <typename> typename Allocator = std::allocator>
    struct default_config{
        using null = null_t;
        using boolean = bool;
        using floating_point = long double;
        using integer = long long;
        using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
        template <typename T>
        using allocator = Allocator<T>;
    };

    static_assert(::netformats::details::json_config<default_config<>>);
}
