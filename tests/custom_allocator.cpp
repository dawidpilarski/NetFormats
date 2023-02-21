/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */


#include "test_defs.hpp"
#include <concepts>
#include <map>
#include <iostream>
#include <typeindex>
#include <cxxabi.h>

std::map<std::type_index , std::size_t> allocation_data;

template <typename T>
class myallocator{
public:
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void *;
    using value_type =  T;
    using size_type = typename std::allocator<T>::size_type;
    using difference_type = typename std::allocator<T>::difference_type;

    myallocator(){
        allocation_data.clear();
    }

    myallocator(const myallocator&) = default;
    myallocator(myallocator&&) = default;

    template <typename U>
    requires (!std::same_as<U, myallocator>)
    myallocator(U&& u) : alloc(std::move(std::forward<U>(u).alloc)){}


    decltype(auto) allocate(size_type n){
        allocation_data[typeid(T)] += n;
        return alloc.allocate(n);
    }

    auto allocate_at_least(size_type n){
        auto result = alloc.allocate_at_least(n);
        allocation_data[typeid(T)] += result.count;

        return result;
    }

    void deallocate(pointer p, size_type n){
        return alloc.deallocate(p, n);
    }

    template <typename U>
    friend class myallocator;

private:
    std::allocator<T> alloc={};
};

template <typename T1, typename T2>
constexpr bool operator==( const myallocator<T1>& lhs, const myallocator<T2>& rhs ) noexcept{
    return true;
}

using myallocatorconfig = netformats::json::custom_allocator_config<myallocator>;
using parser = netformats::json::basic_parser<myallocatorconfig>;

std::string demangle(const char* name){
    std::string output;
    output.reserve(2048);
    std::size_t length=2048;
    int status;
    return std::string(abi::__cxa_demangle(name, output.data(), &length, &status));

}

TEST_CASE("Custom allocator"){
    const std::string json_string = R"({
"property": "string",
"property10": "longlongstringthatneedstobeallocated",
"property2": {
  "nested_property": ["string", 1, 2, 3]
}
})";

    parser p;
    auto result = p.parse(json_string);


    // dynamically allocated for array when allocations occur in order 1, 2, 4,
    // which sums up to 7. Optimal number of allocations would be 4
    // CHECK(allocation_data[&typeid(parser::value)] == 7);
    CHECK(allocation_data[typeid(parser::value)] > 4);

    // not possible to check exact number of rbtree nodes allocated
    // since node_type in the assoc containers are wrappers on top of the
    // rtbtree nodes
    CHECK(allocation_data.contains(typeid(char)));

    for(const auto& [key, value] : allocation_data){
        CAPTURE(std::pair{demangle(key.name()), value});
    }

    auto root = result.get_if<parser::object>();
    REQUIRE(root != nullptr);

    REQUIRE(root->has_member_of_type<parser::string>("property"));
    REQUIRE(root->has_member_of_type<netformats::json::json_type::object>("property2"));

    CHECK(root->get_member("property").get<parser::string>() == "string");
    CHECK(root->get_member("property2").get<parser::object>() ==
            parser::object{
                {"nested_property", parser::array{{parser::string{"string"},
                                                           parser::integer{1},
                                                           parser::integer{2},
                                                           parser::integer{3}}}}
            });

}