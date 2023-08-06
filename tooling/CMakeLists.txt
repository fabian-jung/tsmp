add_library(introspect_lib STATIC 
    data/types.cpp
    data/renderer.cpp
    engine/ast_traversal_tool.cpp
    engine/utils.cpp
)
target_link_libraries(introspect_lib PUBLIC LibClangCpp fmt::fmt)
target_compile_features(introspect_lib PUBLIC cxx_std_17)
target_include_directories(introspect_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(introspect_tool bin/introspect.cpp)
target_link_libraries(introspect_tool PUBLIC introspect_lib)
target_compile_options(introspect_tool PRIVATE ${TSMP_CMAKE_CXX_FLAGS})

include(../cmake/enable_reflection.cmake)