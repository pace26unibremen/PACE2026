add_subdirectory(Graph)
add_subdirectory(Cluster)
add_subdirectory(Solver)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Solver Graph Cluster)
target_compile_definitions(main PRIVATE RES_DIR="${CMAKE_SOURCE_DIR}/res/")

add_executable(solver solver.cpp)
target_link_libraries(solver PRIVATE Solver Graph Cluster)