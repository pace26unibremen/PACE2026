file (GLOB logger_source *.cpp)
file (GLOB logger_header *.hpp)

add_library(Logger ${logger_source})

set_target_properties(Logger PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${logger_header}")

#cmake_policy(SET CMP0167 NEW)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS log log_setup)
include_directories(${Boost_INCLUDE_DIRS})

target_compile_definitions(Logger PRIVATE LOG_FILE_PATH="${CMAKE_SOURCE_DIR}/.log"
                                          LOG_FILTER_PATH="${CMAKE_SOURCE_DIR}/log_filter.ini")
target_include_directories(Logger PUBLIC)
target_link_libraries(Logger PRIVATE ${Boost_LIBRARIES})
