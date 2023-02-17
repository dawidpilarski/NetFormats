/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <vector>

namespace netformats::json{

template<typename value, typename allocator = std::allocator<value>>
class basic_array : public std::vector<value, allocator> {
public:
    using std::vector<value>::vector;
};

}