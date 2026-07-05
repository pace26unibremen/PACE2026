add_subdirectory(Graph)
add_subdirectory(Solver)

# startSolver runs the tighter lower-bound dual on a background thread (std::thread).
find_package(Threads REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Solver Graph)
target_compile_definitions(main PRIVATE RES_DIR="${CMAKE_SOURCE_DIR}/res/")

add_executable(startSolver startSolver.cpp)
target_link_libraries(startSolver PRIVATE Solver Graph Threads::Threads)