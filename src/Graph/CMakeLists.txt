file (GLOB graph_source *.cpp)
file (GLOB graph_header *.hpp)

add_library(Graph ${graph_source})


set_target_properties(Graph PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${graph_header}")
