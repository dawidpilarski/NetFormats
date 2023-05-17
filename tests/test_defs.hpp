/*
 * Copyright (c) 2023 Dawid Pilarski
 * BSD 2-Clause license.
 */


#pragma once

#include <netformats/json/basic_parser.hpp>
#include <netformats/json/json_config.hpp>
#include <netformats/null.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <iostream>

using default_parser = netformats::json::basic_parser<netformats::json::default_config>;

inline void diff(std::string_view str1, std::string_view str2){
    std::string common;
    std::string_view smaller = (str1.length() > str2.length()) ? str2 : str1;

    std::size_t idx=0;
    for(; idx < smaller.size(); ++idx){
        if(str1[idx] != str2[idx]){
            std::cout << "[diff]: " << (int) str1[idx] << " vs " << (int) str2[idx] << std::endl;
            break;
        }

        common.push_back(str1[idx]);
    }

    std::cout << "[1,2]" <<  common << std::endl;
    std::cout << "[1] " << std::string_view{str1.data()+idx, str1.end()} << std::endl;
    std::cout << "[2] " << std::string_view{str2.data()+idx, str2.end()} << std::endl;
}
