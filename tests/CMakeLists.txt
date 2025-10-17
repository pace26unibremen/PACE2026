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

add_executable(TreeIOTests TreeIOTests.cpp)
target_link_libraries(TreeIOTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME TreeIOTests COMMAND $<TARGET_FILE:TreeIOTests>)
target_compile_definitions(TreeIOTests PRIVATE TEST_EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/res/examples/")

add_executable(TreeContractTests TreeContractTests.cpp)
target_link_libraries(TreeContractTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME TreeContractTests COMMAND $<TARGET_FILE:TreeContractTests>)

add_executable(TreeEqualTests TreeEqualTests.cpp)
target_link_libraries(TreeEqualTests PRIVATE Graph Catch2::Catch2WithMain)
add_test(NAME TreeEqualTests COMMAND $<TARGET_FILE:TreeEqualTests>)