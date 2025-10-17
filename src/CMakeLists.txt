add_subdirectory(Logger)
add_subdirectory(Graph)
#add_subdirectory(Solver)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Graph Logger)
target_compile_definitions(main PRIVATE RES_DIR="${CMAKE_SOURCE_DIR}/res/")