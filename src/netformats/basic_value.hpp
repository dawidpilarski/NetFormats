/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <variant>
#include <optional>
#include <concepts>
#include <utility>
#include <string>
#include <stdexcept>
#include <memory>
#include <optional>
#include <type_traits>

#include <netformats/storage_defs.hpp>
#include <netformats/null.hpp>
#include <netformats/type_traits.hpp>

namespace netformats::json {

template<typename key,
        typename value,
        typename storage>
class basic_object;

template<typename value>
class basic_array;


enum class json_type : std::size_t {
    null,
    boolean,
    floating_point,
    integer,
    string,
    array,
    object
};

constexpr std::string to_string(json_type type){
    switch (type) {
        case json_type::null:
            return "null";
        case json_type::boolean:
            return "boolean";
        case json_type::floating_point:
            return "floating point";
        case json_type::integer:
            return "integer";
        case json_type::string:
            return "string";
        case json_type::array:
            return "array";
        case json_type::object:
            return "object";
    }

    throw std::invalid_argument("Unknown json type");
}

namespace details{
    template <typename basic_value, json_type>
    class idx_to_type;
}

template <netformats::details::json_config config>
class basic_value{
public:
    using null = typename config::null;
    using boolean = typename config::boolean;
    using floating_point = typename config::floating_point;
    using integer = typename config::integer;
    using string = typename config::string;
    using array = basic_array<basic_value>;
    using object = basic_object<typename config::string, basic_value, storages::random_order_no_duplicates<typename config::string, basic_value>>;

private:
    using variant = std::variant<
            null,
            boolean,
            floating_point,
            integer,
            string,
            array ,
            object>;

public:



    template <typename T>
    struct can_store{
        constexpr static inline bool value = ::netformats::details::is_one_of<T, null, boolean, floating_point , integer, string, array, object>;
    };

    template <typename T>
    constexpr static inline bool can_store_v = can_store<T>::value;


    template <typename T>
    requires(can_store_v<T>)
    constexpr json_type type_to_idx(){
        if constexpr (std::same_as<T, null>) return json_type::null;
        else if constexpr (std::same_as<T, boolean>) return json_type::boolean;
        else if constexpr (std::same_as<T, floating_point>) return json_type::floating_point;
        else if constexpr (std::same_as<T, integer>) return json_type::integer;
        else if constexpr (std::same_as<T, string>) return json_type::string;
        else if constexpr (std::same_as<T, array>) return json_type::array;
        else if constexpr (std::same_as<T, object>) return json_type::object;
        else {
            static_assert(::netformats::details::always_false<T>::value, "unrecognized type");
        }
    }

    template <json_type idx>
    using idx_to_type = details::idx_to_type<basic_value, idx>;

    template <json_type idx>
    using idx_to_type_t = typename idx_to_type<idx>::type;

    template <typename T>
            requires (can_store_v<T>)
    using in_place_type_t = std::in_place_type_t<T>;

    template <typename T>
    static constexpr inline in_place_type_t<T> in_place_type{};

    template <json_type idx>
    struct in_place_index_t{
        static constexpr inline json_type value = idx;
    };

    template <json_type idx>
    static constexpr inline in_place_index_t<idx> in_place_index{};

    // ctors and assignments

    constexpr basic_value() noexcept : value_(std::in_place_index_t<static_cast<std::size_t>(type_to_idx<null>())>{}, std::nullopt){}
    template <typename T>
    requires (can_store_v<std::remove_cvref_t<T>>)
    constexpr basic_value(T&& value)  : value_(std::forward<T>(value)) {}

    constexpr basic_value(const basic_value&) = default;
    constexpr basic_value(basic_value&&) noexcept(std::is_nothrow_move_constructible_v<variant>) = default;

    constexpr basic_value& operator=(const basic_value&) = default;
    constexpr basic_value& operator=(basic_value&&) noexcept(noexcept(std::is_nothrow_move_assignable_v<variant>)) = default;

    template <typename T>
    requires (can_store_v<std::remove_cvref_t<T>>)
    constexpr basic_value& operator=(T&& value){value_ = std::forward<T>(value);}

    // in-place ctors

    template <typename T, typename... Args>
    requires(can_store_v<T> && std::constructible_from<T, Args...>)
    constexpr explicit(sizeof...(Args) == 0) basic_value(in_place_type_t<T> in_place, Args&&... args) : value_(std::in_place_type<T>, std::forward<Args>(args)...){}

    template <typename T, typename U, typename... Args>
    requires(can_store_v<T> && std::constructible_from<T, std::initializer_list<U>, Args...>)
    constexpr basic_value(in_place_type_t<T> in_place, std::initializer_list<U> il, Args&&... args) : value_(std::in_place_type<T>, il, std::forward<Args>(args)...){}

    template <json_type idx, typename... Args>
    requires(std::constructible_from<typename idx_to_type<idx>::type, Args...>)
    constexpr explicit(sizeof...(Args) == 0) basic_value(in_place_index_t<idx> in_place, Args&&... args) : value_(std::in_place_index<static_cast<std::size_t>(idx)>, std::forward<Args>(args)...){}

    template <json_type idx, typename U, typename... Args>
    requires(std::constructible_from<typename idx_to_type<idx>::type, std::initializer_list<U>, Args...>)
    constexpr basic_value(std::in_place_index_t<static_cast<std::size_t>(idx)> in_place, std::initializer_list<U> il, Args&&... args) : value_(std::in_place_index<static_cast<int>(idx)>, il, std::forward<Args>(args)...){}

    // functions

    [[nodiscard]] constexpr json_type index() const noexcept {return static_cast<json_type>(value_.index());}
    [[nodiscard]] constexpr bool valueless_by_exception() const noexcept {return value_.valueless_by_exception();}

    template <class T, class... Args>
    requires(can_store_v<T> && std::constructible_from<T, Args...>)
    constexpr T& emplace(Args&&... args){
        return value_.emplace(std::forward<Args>(args)...);
    }

    template <class T, class U, class... Args>
    requires(can_store_v<T> && std::constructible_from<T, std::initializer_list<U>, Args...>)
    constexpr T& emplace( std::initializer_list<U> il, Args&&... args ){
        return value_.emplace(il, std::forward<Args>(args)...);
    }

    template <json_type idx, class... Args>
    requires(static_cast<std::size_t>(idx) < std::variant_size_v<variant> && std::constructible_from<std::variant_alternative_t<static_cast<std::size_t>(idx), variant>, Args...>)
    constexpr idx_to_type_t<idx>& emplace( Args&&... args ){
        return value_.emplace(std::forward<Args>(args)...);
    }

    template <json_type idx, class U, class... Args>
    requires(static_cast<std::size_t>(idx) < std::variant_size_v<variant>, std::constructible_from<std::variant_alternative_t<static_cast<std::size_t>(idx), variant>, std::initializer_list<U>, Args...>)
    constexpr idx_to_type_t<idx>&
    emplace( std::initializer_list<U> il, Args&&... args ){
        return value_.emplace(il, std::forward<Args>(args)...);
    }

    constexpr void swap(basic_value& other) noexcept(std::declval<variant&>().swap(std::declval<variant&>())){
        value_.swap(other.value_);
    }

    template <typename U>
    requires (can_store_v<U>)
    constexpr bool holds_alternative(){
        return std::holds_alternative<U>(value_);
    }

    template <typename U>
    requires (can_store_v<U>)
    constexpr U& get() & {
        return std::get<U>(value_);
    }

    template <typename U>
    requires (can_store_v<U>)
    constexpr const U& get() const & {
        return std::get<U>(value_);
    }

    template <typename U>
    requires (can_store_v<U>)
    constexpr U& get() && {
        return std::get<U>(std::move(value_));
    }

    template <json_type idx>
    constexpr idx_to_type_t<idx>& get() & {
        return std::get<static_cast<std::size_t>(idx)>(value_);
    }

    template <json_type idx>
    constexpr const idx_to_type_t<idx>& get() const & {
        return std::get<static_cast<std::size_t>(idx)>(value_);
    }

    template <json_type idx>
    constexpr idx_to_type_t<idx>&& get() && {
        return std::get<static_cast<std::size_t>(idx)>(std::move(value_));
    }

    template <typename U>
    requires can_store_v<U>
    constexpr U* get_if() &{
        return std::get_if<U>(value_);
    }

    template <typename U>
    requires can_store_v<U>
    constexpr U const * get_if() const &{
        return std::get_if<U>(value_);
    }

    template <typename U>
    requires can_store_v<U>
    constexpr U* get_if() &&{
        return std::get_if<U>(value_);
    }

    template <json_type idx>
    constexpr idx_to_type_t<idx>* get_if() &{
        return std::get_if<idx>(&value_);
    }

    template <json_type idx>
    constexpr idx_to_type_t<idx> const * get_if() const &{
        return std::get_if<idx>(&value_);
    }

    template <json_type idx>
    constexpr idx_to_type_t<idx>* get_if() && {
        return std::get_if<idx>(&value_);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) & {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const & {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) && {
        return std::visit(std::forward<Visitor>(visitor), std::move(value_));
    }

    friend bool operator==(const basic_value& left, const basic_value& right) {
        if(left.index() != right.index()) return false;

        if(left.valueless_by_exception()) return true;

        switch(left.index()){
            case json_type::null:
                return true;
            case json_type::floating_point:
                return left.get<json_type::floating_point>() == right.get<json_type::floating_point>();
            case json_type::integer:
                return left.get<json_type::integer>() == right.get<json_type::integer>();
            case json_type::boolean:
                return left.get<json_type::boolean>() == right.get<json_type::boolean>();
            case json_type::string:
                return left.get<json_type::string>() == right.get<json_type::string>();
            case json_type::object:
                return left.get<json_type::object>() == right.get<json_type::object>();
            case json_type::array:
                return left.get<json_type::array>() == right.get<json_type::array>();
        }

        throw std::invalid_argument("unexpected stored type");
    }

    constexpr variant& native_handle() & {return value_;}
    constexpr variant&& native_handle() && {return std::move(value_);}
    constexpr const variant& native_handle() const & {return value_;}
private:
    variant value_{};
};

namespace details{
template<typename basic_value>
struct idx_to_type<basic_value, json_type::null>{
    using type = typename basic_value::null;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::boolean>{
    using type = typename basic_value::boolean;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::floating_point>{
    using type = typename basic_value::floating_point;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::integer>{
    using type = typename basic_value::integer;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::string>{
    using type = typename basic_value::string;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::array>{
    using type = typename basic_value::array;
};

template<typename basic_value>
struct idx_to_type<basic_value, json_type::object>{
    using type = typename basic_value::object;
};
}


}