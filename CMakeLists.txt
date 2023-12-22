cmake_minimum_required (VERSION 3.10)
project(jsonrealize_test CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

add_library(jsonrealize json.cpp)
add_executable(jsonrealize_test test.cpp)
target_link_libraries(jsonrealize_test jsonrealize)
