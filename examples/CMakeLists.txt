function(enable_reflection target)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/examples/simple.cpp.dir/reflection.hpp
        COMMAND ./bin/introspect_tool ../examples/simple.cpp ./examples/simple.cpp.dir/reflection.hpp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS introspect_tool
        DEPENDS simple.cpp
    )
    add_custom_target(simple.cpp.header
        DEPENDS ${CMAKE_BINARY_DIR}/examples/simple.cpp.dir/reflection.hpp
    )
    target_link_libraries(simple PUBLIC simple_cpp_header)
endfunction()


add_library(simple_cpp_header INTERFACE)
add_dependencies(simple_cpp_header simple.cpp.header)
target_include_directories(simple_cpp_header INTERFACE ${CMAKE_BINARY_DIR}/examples/simple.cpp.dir)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
add_executable(simple simple.cpp)
target_link_libraries(simple PUBLIC tsmp)
enable_reflection(simple)
