#
#  Copyright (c) 2023 Dawid Pilarski
#  BSD 2-Clause license.
#

project(NFlib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE
        netformats/type_traits.hpp
        netformats/expected.hpp
        netformats/text_position.hpp
        netformats/unicode_tokenizer.hpp
        netformats/json/parse_error.hpp
        netformats/json/storage_defs.hpp
        netformats/json/basic_object.hpp
        netformats/json/basic_array.hpp
        netformats/json/basic_value.hpp
        netformats/json/basic_parser.hpp
        netformats/json/stringify.hpp
        netformats/json/constants.hpp
        netformats/json/tokenizer.hpp)
target_include_directories(${PROJECT_NAME} INTERFACE .)