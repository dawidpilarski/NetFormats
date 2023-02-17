/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <vector>
#include <list>

namespace netformats::json{

template<typename value>
class basic_array : public std::vector<value> {
public:
    using std::vector<value>::vector;
};

}