/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <netformats/null.hpp>
#include <netformats/type_traits.hpp>
#include "storage_defs.hpp"

#include <string>
#include <scoped_allocator>

namespace netformats::json{

    struct default_config{
        using null = null_t;
        using boolean = bool;
        using floating_point = long double;
        using integer = long long;

        template <typename T>
        using allocator = std::allocator<T>;

        using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;

        template <typename value>
        using storage = storages::random_order_no_duplicates<string, value, allocator<std::pair<const string, value>>>;
    };

    static_assert(::netformats::details::json_config<default_config>);

    template <template <typename> typename Allocator = std::allocator>
    struct custom_allocator_config{
        using null = null_t;
        using boolean = bool;
        using floating_point = long double;
        using integer = long long;

        template <typename T>
        using allocator = Allocator<T>;

        using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;
        template <typename value>
        using storage = storages::alphabetical_order_no_duplicates<string, value, allocator<std::pair<const string, value>>>;
    };
}
