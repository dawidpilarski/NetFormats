/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */

#pragma once

#include <optional>

class null_t{
public:
    null_t([[maybe_unused]] const std::nullopt_t& null){};
    null_t() = default;
    null_t(const null_t&) = default;
    null_t(null_t&&) = default;

    null_t& operator=(const null_t&) = default;
    null_t& operator=(null_t&&) = default;

    auto operator <=>(const null_t&) const = default;
};

static inline const null_t null{};