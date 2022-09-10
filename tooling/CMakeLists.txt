set(introspect_lib_src
    data/aggregator.cpp
    data/types.cpp
    data/prefix_splitter.cpp
    data/enum_splitter.cpp
    engine/introspect_visitor.cpp
    engine/utils.cpp
    engine/consumer.cpp
    engine/frontendaction.cpp
)

add_library(introspect_lib STATIC ${introspect_lib_src})
target_link_libraries(introspect_lib PUBLIC LibClangCpp fmt::fmt)
target_compile_features(introspect_lib PUBLIC cxx_std_17)
target_include_directories(introspect_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(introspect_tool bin/introspect.cpp)
target_link_libraries(introspect_tool PUBLIC introspect_lib)
target_compile_options(introspect_tool PRIVATE ${TSMP_CMAKE_CXX_FLAGS})

include(../cmake/enable_reflection.cmake)