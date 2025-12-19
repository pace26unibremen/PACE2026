file (GLOB rule_source *.cpp)
#file (GLOB rule_header *.hpp)
target_sources(Solver PRIVATE ${rule_source})
