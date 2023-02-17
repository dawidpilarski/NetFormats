/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */


#pragma once

#include <netformats/basic_parser.hpp>
#include <netformats/null.hpp>
#include <netformats/json_config.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using default_parser = netformats::json::basic_parser<netformats::json::default_config<>>;