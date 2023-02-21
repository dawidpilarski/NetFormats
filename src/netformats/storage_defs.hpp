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

    using key_type = typename container::key_type;
    using mapped_type = typename container::mapped_type;
    using value_type = typename container::value_type;
    using size_type	 = typename container::size_type;
    using difference_type = typename container::difference_type;
    using key_compare = typename container::key_compare;
    using allocator_type = typename container::allocator_type;
    using reference	= typename container::reference;
    using const_reference = typename container::const_reference;
    using pointer = typename container::pointer;
    using const_pointer = typename container::const_pointer;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

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

    using key_type = typename container::key_type;
    using mapped_type = typename container::mapped_type;
    using value_type = typename container::value_type;
    using size_type	 = typename container::size_type;
    using difference_type = typename container::difference_type;
    using key_compare = typename container::key_compare;
    using allocator_type = typename container::allocator_type;
    using reference	= typename container::reference;
    using const_reference = typename container::const_reference;
    using pointer = typename container::pointer;
    using const_pointer = typename container::const_pointer;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

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

    using key_type = typename container::key_type;
    using mapped_type = typename container::mapped_type;
    using value_type = typename container::value_type;
    using size_type	 = typename container::size_type;
    using difference_type = typename container::difference_type;
    using key_compare = typename container::key_compare;
    using allocator_type = typename container::allocator_type;
    using reference	= typename container::reference;
    using const_reference = typename container::const_reference;
    using pointer = typename container::pointer;
    using const_pointer = typename container::const_pointer;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

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
    using key_type = typename container::key_type;
    using mapped_type = typename container::mapped_type;
    using value_type = typename container::value_type;
    using size_type	 = typename container::size_type;
    using difference_type = typename container::difference_type;
    using key_equal = typename container::key_equal;
    using hasher = typename container::hasher;
    using allocator_type = typename container::allocator_type;
    using reference	= typename container::reference;
    using const_reference = typename container::const_reference;
    using pointer = typename container::pointer;
    using const_pointer = typename container::const_pointer;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;


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

    using key_type = typename container::key_type;
    using mapped_type = typename container::mapped_type;
    using value_type = typename container::value_type;
    using size_type	 = typename container::size_type;
    using difference_type = typename container::difference_type;
    using key_equal = typename container::key_equal;
    using hasher = typename container::hasher;
    using allocator_type = typename container::allocator_type;
    using reference	= typename container::reference;
    using const_reference = typename container::const_reference;
    using pointer = typename container::pointer;
    using const_pointer = typename container::const_pointer;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

    static constexpr bool stores_duplicates = true;
    static constexpr bool is_transparent = false;
}; // unordered multimap

}
