cmake_minimum_required(VERSION 3.27.0)
project(Pace2026 VERSION 0.1.0)
include(CMakePrintHelpers)

message(STATUS "CMake Version: ${CMAKE_VERSION}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall")

add_subdirectory(src)
add_subdirectory(tests)
