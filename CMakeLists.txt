#
#  Copyright (c) 2023 Dawid Pilarski
#  BSD 2-Clause license.
#


cmake_minimum_required(VERSION 3.15)
project(NetFormats)
option(NF_BUILD_TESTS "Build tests" ON)


add_subdirectory(src)

if(NF_BUILD_TESTS)
    add_subdirectory(tests)
endif()