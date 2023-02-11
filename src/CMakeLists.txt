#
#  Copyright (c) 2023 Dawid Pilarski
#  BSD 2-Clause license.
#

project(NFlib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE netformats/unicode_tokenizer.hpp
        netformats/storage_defs.hpp
        netformats/basic_object.hpp
        netformats/basic_array.hpp
        netformats/basic_value.hpp
        netformats/basic_parser.hpp)
target_include_directories(${PROJECT_NAME} INTERFACE .)