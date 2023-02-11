#
#  Copyright (c) 2023 Dawid Pilarski
#  BSD 2-Clause license.
#

project(tests)

enable_testing()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include(CTest)
include(Catch)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} simple_array.cpp simple_root.cpp simple_object.cpp whitespaces.cpp numbers.cpp strings.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC NFlib Catch2Main Catch2 )
catch_discover_tests(${PROJECT_NAME})