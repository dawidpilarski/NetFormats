/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */


#pragma once
#include <type_traits>
#include <concepts>

namespace netformats::details {

    template <typename T, typename... Types>
    concept is_one_of = (std::is_same_v<T, Types> || ...);

    template<typename T>
    struct always_false : std::false_type {
    };

    template<typename T>
    concept is_iterable = requires(T x){
        std::begin(x);
        std::end(x);
        std::cbegin(x);
        std::cend(x);
    };

    template<typename T>
    concept is_reverse_iterable = is_iterable<T> && requires(T x, const T &cx){
        std::cbegin(x);
        std::cend(x);
    };
}