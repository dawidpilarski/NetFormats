cmake_minimum_required(VERSION 3.24)
project(NetFormats)
option(NF_BUILD_TESTS "Build tests" ON)


add_subdirectory(src)

if(NF_BUILD_TESTS)
    add_subdirectory(tests)
endif()