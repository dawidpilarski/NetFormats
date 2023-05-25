/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */


#pragma once

#include <netformats/null.hpp>

#include <utility>
#include <initializer_list>
#include <concepts>
#include <optional>
#include <cassert>

#if __cpp_lib_expected >= 202202L
#include <exception>
#endif

namespace netformats{


class no_value_t{
public:
    no_value_t([[maybe_unused]] const std::nullopt_t& null){};
    no_value_t() = default;
    no_value_t(const no_value_t&) = default;
    no_value_t(no_value_t&&) = default;

    no_value_t& operator=(const no_value_t&) = default;
    no_value_t& operator=(no_value_t&&) = default;

    auto operator <=>(const no_value_t&) const = default;
};

static inline const no_value_t no_value{};


#if __cpp_lib_expected >= 202202L
    template<typename T, typename E>
    using expected = std::expected<T, E>;

    using unexpect_t = std::unexpect_t;

    template<typename E>
    using unexpected = std::unexpected<E>;
#else

    template <typename E>
    requires (
            std::is_object_v<E> &&
            !std::is_array_v<E> &&
            !std::is_const_v<E> &&
            !std::is_volatile_v<E>
                    )
    class unexpected{
        template <typename U>
        struct is_unexpected : std::false_type{};

        template<typename Err>
        struct is_unexpected<unexpected<Err>> : std::true_type{};

    public:

        static_assert(is_unexpected<E>::value == false,
                "E type in specialization of unexpected, cannot be unexpected itself");

        constexpr unexpected(const unexpected&) = default;
        constexpr unexpected(unexpected&&) = default;

        template <typename Err>
        requires(
                !std::same_as<std::remove_cvref_t<Err>, unexpected> &&
                !std::same_as<std::remove_cvref_t<Err>, std::in_place_t> &&
                std::constructible_from<E, Err>)
        constexpr explicit unexpected(Err&& e) : err(std::forward<Err>(e)){}

        template <typename... Args>
        requires(std::constructible_from<E, Args...>)
        constexpr explicit unexpected([[maybe_unused]] std::in_place_t in_place, Args&&... args) : err(std::forward<Args>(args)...){}

        template <typename U, typename... Args>
        requires(std::constructible_from<E, std::initializer_list<U>&, Args...>)
        constexpr explicit unexpected([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> il, Args&&... args) :
        err(il, std::forward<Args>(args)...){}

        constexpr const E& error() const& noexcept{
            return err;
        }
        constexpr E& error() & noexcept{
            return err;
        }
        constexpr const E&& error() const&& noexcept{
            return std::move(*this).err;
        }
        constexpr E&& error() && noexcept{
            return std::move(*this).err;
        }

        constexpr void swap( unexpected& other ) noexcept(std::is_nothrow_swappable_v<E>) requires(std::swappable<E>){
            using std::swap;
            swap(err, other.err);
        }

        template< class E2 >
        friend constexpr bool operator==( unexpected& x, unexpected<E2>& y ){
            return x.error() == y.error();
        }

        friend constexpr void
        swap( unexpected& x, unexpected& y ) noexcept(noexcept(x.swap(y))){
            x.swap(y);
        }

    private:
        E err;
    };

    template< class E >
    unexpected(E) -> unexpected<E>;

    struct unexpect_t{};
    static inline unexpect_t unexpect;


    template <typename E>
    class bad_expected_access;

    template<>
    class bad_expected_access<void> : public std::exception{
    public:
        [[nodiscard]] const char * what() const noexcept override {
            return "Tried to reference value inside the expected type, which contains unexpected error";
        }
    };

    template <typename E>
    class bad_expected_access: public bad_expected_access<void>{
    public:
        explicit bad_expected_access(E err) :
        error_(std::move(err)){}

        [[nodiscard]] const char * what() const noexcept override {
            return "Tried to reference value inside the expected type, which contains unexpected error";
        }

        const E& error() const& noexcept{
            return error_;
        }
        E& error() & noexcept{
            return error_;
        }

        const E&& error()  const&& noexcept{
            return std::move(error_);
        }

        E&& error() && noexcept{
            return std::move(error_);
        }

    private:
        E error_;
    };


    template <std::destructible T, std::destructible E>
    requires requires(){
        requires(!std::same_as<std::remove_cv_t<T>, void>);
        typename unexpected<E>;
    }
    class expected{
    public:
        using value_type = T;
        using error_type = E;
        using unexpected_type = unexpected<E>;

        constexpr expected() noexcept(std::is_nothrow_default_constructible_v<value_type>) :
        storage(){
            construct_value();
        }

        constexpr expected(const expected& other) requires(std::copy_constructible<value_type> && std::copy_constructible<error_type>) : storage() {
            if(other.has_value()){
                construct_value(*other);
            } else {
                construct_error(other.error());
            }
        }


        constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_move_constructible_v<error_type>){
            if(other.has_value()){
                construct_value(std::move(*other));
            } else {
                construct_error(std::move(other.error()));
            }
        }

        template <typename U = value_type>
        requires(!std::same_as<std::in_place_t, std::remove_cvref_t<U>> &&
                 !std::same_as<expected, std::remove_cvref_t<U>> &&
                 std::is_constructible_v<value_type, U>)
        constexpr explicit(!std::is_convertible_v<U, value_type>) expected(U&& v) : storage() {
            construct_value(std::forward<U>(v));
        }

        template <typename G = error_type>
        constexpr explicit(!std::is_convertible_v<G, error_type>) expected(const unexpected<G>& err){
            construct_error(err.error());
        }

        template <typename G = error_type>
        constexpr explicit(!std::is_convertible_v<G, error_type>) expected(unexpected<G>&& err){
            construct_error(std::move(err).error());
        }

        template <typename... Args>
        requires (std::constructible_from<value_type, Args...>)
        constexpr explicit expected(std::in_place_t, Args&&... args) : storage(){
            construct_value(std::forward<Args>(args)...);
        }

        template <typename U, typename... Args>
        requires (std::constructible_from<value_type, std::initializer_list<U>, Args...>)
        constexpr explicit expected(std::in_place_t,
                                    std::initializer_list<U> il,
                                    Args&&... args): storage(){
            construct_value(il, std::forward<Args>(args)...);
        }

        template <typename... Args>
        requires (std::constructible_from<error_type, Args...>)
        constexpr explicit expected(unexpect_t, Args&&... args) : storage(){
            construct_error(std::forward<Args>(args)...);
        }

        template <typename U, typename... Args>
        requires (std::constructible_from<error_type, std::initializer_list<U>, Args...>)
        constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : storage(){
            construct_error(il, std::forward<Args>(args)...);
        }

        constexpr explicit operator bool() const noexcept{
            return discriminator == Discriminator::VALUE;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return static_cast<bool>(*this);
        }

        constexpr value_type& value() &{
            if(!has_value()){
                throw bad_expected_access{error()};
            }
            return storage.value;
        }

        constexpr const value_type& value() const &{
            if(!has_value()){
                throw bad_expected_access{error()};
            }
            return storage.value;
        }

        constexpr const value_type& value() &&{
            if(!has_value()){
                throw bad_expected_access{std::move(error())};
            }
            return storage.value;
        }

        constexpr const value_type& value() const &&{
            if(!has_value()){
                throw bad_expected_access{std::move(error())};
            }
            return storage.value;
        }

        constexpr error_type& error() &{
            assert(!has_value());
            return storage.error;
        }

        constexpr const error_type& error() const &{
            assert(!has_value());
            return storage.error;
        }

        constexpr const error_type& error() &&{
            assert(!has_value());
            return storage.error;
        }

        constexpr const error_type& error() const &&{
            assert(!has_value());
            return storage.error;
        }

        constexpr value_type& operator*() & noexcept{
            return value();
        }

        constexpr const value_type& operator*() const & noexcept{
            return value();
        }

        constexpr value_type&& operator*() && noexcept{
            return std::move(value());
        }

        constexpr const value_type&& operator*() const && noexcept{
            return std::move(value());
        }

        constexpr const value_type* operator->() const noexcept{
            return &value();
        }

        constexpr value_type* operator->() noexcept{
            return &value();
        }

        template <typename U>
        requires(std::copy_constructible<T> && std::convertible_to<U, T>)
        constexpr T value_or(U&& default_value) const&{
            has_value() ? **this : static_cast<T>(std::forward<U>(default_value));
        }

        template <typename U>
        requires(std::move_constructible<T> && std::convertible_to<U, T>)
        constexpr T value_or(U&& default_value) &&{
            has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
        }

        template <typename U>
        requires(std::copy_constructible<error_type> && std::convertible_to<U, error_type>)
        constexpr error_type error_or(U&& default_value) const&{
            !has_value() ? error() : static_cast<error_type>(std::forward<U>(default_value));
        }

        template <typename U>
        requires(std::move_constructible<error_type> && std::convertible_to<U, error_type>)
        constexpr error_type error_or(U&& default_value) &&{
            !has_value() ? std::move(error()) : static_cast<error_type>(std::forward<U>(default_value));
        }

        constexpr ~expected() {
            if(has_value()){
                std::destroy_at(storage_as_value());
            } else {
                std::destroy_at(storage_as_error());
            }
        }



    private:

        template <typename... Args>
        constexpr void construct_value(Args&&... args){
            std::construct_at(storage_as_value(), std::forward<Args>(args)...);
            discriminator = Discriminator::VALUE;
        }

        template <typename... Args>
        constexpr void construct_error(Args&&... args){
            std::construct_at(storage_as_error(), std::forward<Args>(args)...);
            discriminator = Discriminator::ERROR;
        }

        constexpr value_type* storage_as_value(){
            return &storage.value;
        }

        constexpr error_type* storage_as_error(){
            return &storage.error;
        }

        union value_error{
            constexpr value_error(){};
            value_type value;
            error_type error;
            constexpr ~value_error(){};
        } storage;
        enum class Discriminator{
            VALUE,
            ERROR
        } discriminator;
    };

    template <typename Error>
    using expected_no_value = expected<null_t, Error>;

#endif
}