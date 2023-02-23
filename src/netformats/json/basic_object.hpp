/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <unordered_map>
#include <ranges>
#include <concepts>
#include <cassert>
#include "storage_defs.hpp"
#include "basic_value.hpp"
#include "netformats/type_traits.hpp"


namespace netformats::json {

template<typename storage>
class basic_object {
public:
    using key_type = typename storage::key_type;
    using mapped_type = typename storage::mapped_type;
    using value_type = typename storage::value_type;
    using size_type	 = typename storage::size_type;
    using difference_type = typename storage::difference_type;
    using allocator_type = typename storage::allocator_type;
    using reference	= typename storage::reference;
    using const_reference = typename storage::const_reference;
    using pointer = typename storage::pointer;
    using const_pointer = typename storage::const_pointer;
    using iterator = typename storage::iterator;
    using const_iterator = typename storage::const_iterator;
    using node_type = typename storage::node_type;

    basic_object() = default;
    basic_object(const basic_object &) = default;
    basic_object(basic_object &&) noexcept(std::is_nothrow_move_constructible_v < storage > ) = default;
    explicit basic_object(allocator_type allocator) noexcept(std::is_nothrow_constructible_v < storage,
                                                                      allocator_type > ): properties(
            std::move(allocator)) {}
    basic_object(std::initializer_list<value_type> init, const allocator_type& alloc = allocator_type{} ) :
    properties(init, alloc){}

    basic_object &operator=(const basic_object &) = default;
    basic_object &operator=(basic_object &&) noexcept(std::is_nothrow_move_assignable_v < storage > ) = default;

    friend bool operator==(const basic_object &left, const basic_object &right) {
        return left.properties == right.properties;
    }

    [[nodiscard]] auto begin() { return std::ranges::begin(properties); }
    [[nodiscard]] auto end() { return std::ranges::end(properties); }
    [[nodiscard]] auto begin() const { return std::ranges::begin(properties); }
    [[nodiscard]] auto end() const { return std::ranges::end(properties); }
    [[nodiscard]] auto cbegin() const { return std::ranges::cbegin(properties); }
    [[nodiscard]] auto cend() const { return std::ranges::cend(properties); }
    [[nodiscard]] auto rbegin() requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::rbegin(properties); }
    [[nodiscard]] auto rend() requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::rend(properties); }
    [[nodiscard]] auto rbegin() const requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::rbegin(properties); }
    [[nodiscard]] auto rend() const requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::rend(properties); }
    [[nodiscard]] auto crbegin() const requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::crbegin(properties); }
    [[nodiscard]] auto crend() const requires(::netformats::details::is_reverse_iterable <storage>) { return std::ranges::crend(properties); }

    //todo handle transparent and non transparent comparisons
    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] auto find(const key_comparable &comparable) const requires (storage::is_transparent) {
        return find_(comparable);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] auto find(const key_comparable &comparable) requires (storage::is_transparent) {
        return find_(comparable);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] bool contains(const key_comparable &comparable) const requires (storage::is_transparent) {
        return properties.find(comparable) != std::ranges::end(properties);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] bool has_member(const key_comparable &comparable) const requires (storage::is_transparent) {
        return contains(comparable);
    }

    template<typename T, std::equality_comparable_with<key_type> key_comparable>
    requires (mapped_type::template can_store_v<T>)
    [[nodiscard]] bool has_member_of_type(const key_comparable &comparable) const requires (storage::is_transparent) {
        auto it = find(comparable);
        if(it == end()) return false;

        return it->second.template get_if<T>() != nullptr;
    }

    template<json_type idx, std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] bool has_member_of_type(const key_comparable &comparable) const requires (storage::is_transparent) {
        auto it = find(comparable);
        if(it == end()) return false;

        return it->second.index() == idx;
    }

    [[nodiscard]] auto find(const key_type& searched_key) const requires (!storage::is_transparent) {
        return find_(searched_key);
    }

    [[nodiscard]] auto find(const key_type& searched_key) requires (!storage::is_transparent) {
        return find_(searched_key);
    }

    [[nodiscard]] bool contains(const key_type& searched_key) const requires (!storage::is_transparent) {
        return properties.find(searched_key) != std::ranges::end(properties);
    }

    [[nodiscard]] bool has_member(const key_type& searched_key) const requires (!storage::is_transparent) {
        return contains(searched_key);
    }

    template<typename T>
    requires (mapped_type::template can_store_v<T>)
    [[nodiscard]] bool has_member_of_type(const key_type &comparable) const requires (!storage::is_transparent) {
        auto it = find(comparable);
        if(it == end()) return false;

        return it->second.template get_if<T>() != nullptr;
    }

    template<json_type idx>
    [[nodiscard]] bool has_member_of_type(const key_type &comparable) const requires (!storage::is_transparent) {
        auto it = find(comparable);
        if(it == end()) return false;

        return it->second.index() == idx;
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] decltype(auto) find_all(const key_comparable& comparable) requires(storage::is_transparent && storage::stores_duplicates){
        return find_all_(comparable);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] decltype(auto) find_all(const key_comparable& comparable) const requires(storage::is_transparent && storage::stores_duplicates){
        return find_all_(comparable);
    }

    [[nodiscard]] decltype(auto) find_all(const key_type& searched_key) requires(!storage::is_transparent && storage::stores_duplicates){
        return find_all_(searched_key);
    }

    [[nodiscard]] decltype(auto) find_all(const key_type& searched_key) const requires(!storage::is_transparent && storage::stores_duplicates){
        return find_all_(searched_key);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] decltype(auto) get_member(const key_comparable& comparable) requires(storage::is_transparent){
        return get_member_(comparable);
    }

    template<std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] decltype(auto) get_member(const key_comparable& comparable) const requires (storage::is_transparent){
        return get_member_(comparable);
    }

    [[nodiscard]] decltype(auto) get_member(const key_type& comparable) requires(!storage::is_transparent){
        return get_member_(comparable);
    }

    [[nodiscard]] decltype(auto) get_member(const key_type& comparable) const requires (!storage::is_transparent){
        return get_member_(comparable);
    }

    template<typename value_t, std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] value_t& get_member(const key_comparable& comparable) requires(storage::is_transparent && mapped_type::template can_store_v<value_t>){
        return get_member_(comparable).template get<value_t>();
    }

    template<typename value_t, std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] const value_t& get_member(const key_comparable& comparable) const requires (storage::is_transparent && mapped_type::template can_store_v<value_t>){
        return get_member_(comparable).template get<value_t>();
    }

    template <typename value_t>
    [[nodiscard]] value_t get_member(const key_type& comparable) requires(!storage::is_transparent && mapped_type::template can_store_v<value_t>){
        return get_member_(comparable).template get<value_t>();
    }

    template <typename value_t>
    [[nodiscard]] decltype(auto) get_member(const key_type& comparable) const requires (!storage::is_transparent && mapped_type::template can_store_v<value_t>){
        return get_member_(comparable).template get<value_t>();
    }

    template <std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] std::optional<json_type> member_type(const key_comparable& comparable) const requires(storage::is_transparent){
        auto it = find(comparable);
        if(it == end()) return {};
        return it->second.index();
    }

    [[nodiscard]] std::optional<json_type> member_type(const key_type& comparable) const requires(!storage::is_transparent){
        auto it = find(comparable);
        if(it == end()) return {};
        return it->second.index();
    }

    template<typename key_comparable, typename value_t>
    auto insert_or_assign(key_comparable&& comparable, value_t&& updated_element)
    requires(storage::is_transparent &&
             std::equality_comparable_with<key_type, key_comparable> &&
             std::constructible_from<key_type, decltype(comparable)> &&
             std::assignable_from<mapped_type, decltype(updated_element)> &&
             std::constructible_from<mapped_type, decltype(updated_element)>){
        auto it = find(comparable);
        if(it == end()){
            it->second = std::forward<value_t>(updated_element);
            return it;
        }

        return properties.emplace(std::forward<key_comparable>(comparable), std::forward<value_t>(updated_element));
    }

    template<typename key_t = key_type, typename value_t>
    auto insert_or_assign(key_type&& comparable, value_t&& updated_element)
    requires(
             !storage::is_transparent &&
             std::same_as<std::remove_cvref_t<key_t>, key_type> &&
             std::assignable_from<mapped_type, decltype(updated_element)> &&
             std::constructible_from<mapped_type, decltype(updated_element)>){
        auto it = find(comparable);
        if(it == end()){
            it->second = std::forward<value_t>(updated_element);
            return it;
        }

        return properties.emplace(std::forward<key_type>(comparable), std::forward<value_t>(updated_element));
    }

    [[nodiscard]] bool empty() const {return properties.empty();}
    [[nodiscard]] auto size() const {return properties.size();}
    [[nodiscard]] auto max_size() const {return properties.max_size();}
    void clear() noexcept {return properties.clear();}

    template <typename T>
    auto insert(const_iterator it, T&& value_) requires(std::constructible_from<value_type , decltype(value_)>){
        return properties.insert(it, std::forward<T>(value_));
    }

    template <typename T>
    auto insert(T&& value_) requires(std::constructible_from<value_type , decltype(value_)>){
        return properties.insert(std::forward<T>(value_));
    }

    template <typename iter>
    auto erase(iter begin, iter end){
        return properties.erase(begin, end);
    }

    template <typename iter>
    auto erase(iter element){
        return properties.erase(element);
    }

    template <std::equality_comparable_with<key_type> key_comparable>
    std::size_t erase(key_comparable&& key_search) requires (storage::is_transparent){
        auto range = find_all(key_search);
        auto distance = std::distance(range.first, range.second);
        properties.erase(range.first, range.second);
        return distance;
    }

    std::size_t erase(const key_type& property_key) requires (!storage::is_transparent){
        auto range = find_all(property_key);
        auto distance = std::distance(range.first, range.second);
        properties.erase(range.first, range.second);
        return distance;
    }

    void swap(basic_object& other) noexcept(std::is_nothrow_swappable_v<basic_object>){
        properties.swap(other.properties);
    }

    template <std::equality_comparable_with<key_type> key_comparable>
    [[nodiscard]] std::size_t count(const key_comparable& searched_comparable) const requires(storage::is_transparent){
        auto it = find_all_(searched_comparable);
        return std::distance(it.first, it.second);
    }

    [[nodiscard]] std::size_t count(const key_type& searched_key) const requires(!storage::is_transparent){
        auto it = find_all_(searched_key);
        return std::distance(it.first, it.second);
    }


    const storage& native_handle() const & {
        return properties;
    }

    storage& native_handle() & {
        return properties;
    }

    storage&& native_handle() && {
        return properties;
    }
private:

    template<std::equality_comparable_with <key_type> key_comparable>
    mapped_type& get_member_(const key_comparable& comparable){
        assert(contains(comparable));
        return find(comparable)->second;
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    const mapped_type& get_member_(const key_comparable& comparable) const {
        assert(contains(comparable));
        return find(comparable)->second;
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    decltype(auto) find_(const key_comparable &searched_key) const {
        return properties.find(searched_key);
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    auto find_(const key_comparable &searched_key) {
        return properties.find(searched_key);
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    auto find_all_(const key_comparable& comparable) requires(storage::stores_duplicates){
        return properties.equal_range(comparable);
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    auto find_all_(const key_comparable& comparable) const requires(storage::stores_duplicates){
        return properties.equal_range(comparable);
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    auto find_all_(const key_comparable& comparable) requires(!storage::stores_duplicates){
        auto it = properties.find(comparable);
        if(it == end()) return std::pair{it, it};
        return std::pair{it, it+1};
    }

    template<std::equality_comparable_with <key_type> key_comparable>
    auto find_all_(const key_comparable& comparable) const requires(!storage::stores_duplicates){
        auto it = properties.find(comparable);
        if(it == end()) return std::pair{it, it};
        return std::pair{it, it+1};
    }

    storage properties;
};

}

