file (GLOB graph_source *.cpp)
file (GLOB graph_header *.hpp)

add_library(Graph ${graph_source}
        LabelTwinRelation.cpp
        LabelTwinRelation.hpp
        InteriorTwinRelation.cpp
        InteriorTwinRelation.hpp
        LeastCommonAncestor.cpp
        LeastCommonAncestor.hpp
        ClusterPointGenerator.cpp
        ClusterPointGenerator.hpp
        ClusterInstance.cpp
        ClusterInstance.hpp
        RecursiveClusterer.cpp
        RecursiveClusterer.hpp)

if (DEBUG_IMAGE_VIEW_GRAPH)
    MESSAGE("-- DEBUG_IMAGE_VIEW_GRAPH is on")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRAPHVIZ REQUIRED libgvc libcgraph)

    find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)

    target_include_directories(Graph
            PUBLIC
            ${GRAPHVIZ_INCLUDE_DIRS}
            ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(Graph
            PUBLIC
            ${GRAPHVIZ_LIBRARIES}
            ${OpenCV_LIBS}
    )
    target_compile_definitions(Graph PUBLIC DEBUG_IMAGE_VIEW_GRAPH)
endif()

set_target_properties(Graph PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${graph_header}")
