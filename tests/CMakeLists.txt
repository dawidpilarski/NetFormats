#
#  Copyright (c) 2023 Dawid Pilarski
#  BSD 2-Clause license.
#

project(tests)

include(CTest)
enable_testing()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include(Catch)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} simple_array.cpp simple_root.cpp simple_object.cpp whitespaces.cpp numbers.cpp strings.cpp test_defs.hpp)
target_compile_options(${PROJECT_NAME} PUBLIC "-ftemplate-backtrace-limit=0")
target_link_libraries(${PROJECT_NAME} PUBLIC NFlib Catch2Main Catch2 )
catch_discover_tests(${PROJECT_NAME})
add_test(NAME unit_tests COMMAND ${PROJECT_NAME})