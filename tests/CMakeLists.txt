#find_package(Catch2)
if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.11.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()
include(CTest)
include(CPack)
enable_testing()

add_executable(ForestIOTests ForestIOTests.cpp)
target_link_libraries(ForestIOTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME ForestIOTests COMMAND $<TARGET_FILE:ForestIOTests>)
target_compile_definitions(ForestIOTests PRIVATE TEST_EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/res/examples/")

add_executable(ForestRemoveEdgeTests ForestRemoveEdgeTests.cpp)
target_link_libraries(ForestRemoveEdgeTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME ForestRemoveEdgeTests COMMAND $<TARGET_FILE:ForestRemoveEdgeTests>)

add_executable(TreeEqualTests TreeEqualTests.cpp)
target_link_libraries(TreeEqualTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME TreeEqualTests COMMAND $<TARGET_FILE:TreeEqualTests>)