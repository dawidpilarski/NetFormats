/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <memory>
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>


namespace netformats::json::storages{

    template<typename key,
            typename value,
            typename allocator = std::allocator <std::pair<key, value>>>
    class preserve_order_store_duplicates : public std::vector<std::pair < key, value>, allocator> {
    public:
    using container = std::vector <std::pair<key, value>, allocator>;
    using container::container;

    using key_type = key;
    using mapped_type = value;
    using value_type = container::value_type;
    using size_type	 = container::size_type;
    using difference_type = container::difference_type;
    using key_compare = container::key_compare;
    using allocator_type = container::allocator_type;
    using reference	= container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    static constexpr bool stores_duplicates = true;
    static constexpr bool is_transparent = true;

};

template<typename key,
        typename value,
        typename allocator = std::allocator <std::pair<const key, value>>>
class alphabetical_order_store_duplicates : public std::multimap<key, value, std::less<>, allocator> {
public:
    using container = std::multimap <key, value, std::less<>, allocator>;
    using container::container;

    using key_type = key;
    using mapped_type = value;
    using value_type = container::value_type;
    using size_type	 = container::size_type;
    using difference_type = container::difference_type;
    using key_compare = container::key_compare;
    using allocator_type = container::allocator_type;
    using reference	= container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    static constexpr bool stores_duplicates = true;
    static constexpr bool is_transparent = true;
};

template<typename key,
        typename value,
        typename allocator = std::allocator <std::pair<const key, value>>>
class alphabetical_order_no_duplicates : public std::map<key, value, std::less<>, allocator> {
public:
    using container = std::map<key, value, std::less<>, allocator>;
    using container::container;

    using key_type = key;
    using mapped_type = value;
    using value_type = container::value_type;
    using size_type	 = container::size_type;
    using difference_type = container::difference_type;
    using key_compare = container::key_compare;
    using allocator_type = container::allocator_type;
    using reference	= container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    static constexpr bool stores_duplicates = false;
    static constexpr bool is_transparent = true;
};

template<typename key,
        typename value,
        typename allocator = std::allocator <std::pair<const key, value>>>
class random_order_no_duplicates : public std::unordered_map<key, value, std::hash<key>, std::equal_to<key>, allocator> {
public:
    using container = std::unordered_map<key, value, std::hash<key>, std::equal_to<key>, allocator>;
    using container::container;
    using key_type = key;
    using mapped_type = value;
    using value_type = container::value_type;
    using size_type	 = container::size_type;
    using difference_type = container::difference_type;
    using key_equal = container::key_equal;
    using hasher = container::hasher;
    using allocator_type = container::allocator_type;
    using reference	= container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;


    static constexpr bool stores_duplicates = false;
    static constexpr bool is_transparent = false;
}; // unordered map

template<typename key,
        typename value,
        typename hash = std::hash <key>,
        typename equal_to = std::equal_to <key>,
        typename allocator = std::allocator <std::pair<const key, value>>>
class random_order_store_duplicates : public std::unordered_multimap<key, value, hash, equal_to, allocator> {
public:
    using container = std::unordered_multimap<key, value, hash, equal_to, allocator>;
    using container::container;

    using key_type = key;
    using mapped_type = value;
    using value_type = container::value_type;
    using size_type	 = container::size_type;
    using difference_type = container::difference_type;
    using key_equal = container::key_equal;
    using hasher = container::hasher;
    using allocator_type = container::allocator_type;
    using reference	= container::reference;
    using const_reference = container::const_reference;
    using pointer = container::pointer;
    using const_pointer = container::const_pointer;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    static constexpr bool stores_duplicates = true;
    static constexpr bool is_transparent = false;
}; // unordered multimap

}
