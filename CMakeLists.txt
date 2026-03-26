cmake_minimum_required(VERSION 3.27.0)

project(Pace2026 VERSION 0.1.0 LANGUAGES C CXX)

include(CMakePrintHelpers)

message(STATUS "CMake Version: ${CMAKE_VERSION}")

option(DEBUG_IMAGE_VIEW_GRAPH
       "Adds option to view graphs as image in debugger"
       OFF)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE}  -fprofile-instr-generate -fcoverage-mapping")
set(CMAKE_CXX_FLAGS_PROFILING  "${CMAKE_CXX_FLAGS_PROFILING}  -O2 -g -gline-tables-only -fdebug-info-for-profiling -funique-internal-linkage-names")

add_subdirectory(src)
add_subdirectory(tests)
