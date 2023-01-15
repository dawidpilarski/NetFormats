
#pragma once

#include <vector>

namespace netformats::json{

template<typename value, typename allocator = std::allocator<value>>
class basic_array : public std::vector<value, allocator> {
public:
    using std::vector<value, allocator>::vector;
};

}