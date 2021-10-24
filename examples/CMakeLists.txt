add_library(simple_cpp_header INTERFACE)
add_dependencies(simple_cpp_header simple.cpp.header)
target_include_directories(simple_cpp_header INTERFACE ${CMAKE_BINARY_DIR}/examples/simple.cpp.dir)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
add_executable(simple ${CMAKE_CURRENT_LIST_DIR}/simple.cpp)
enable_reflection(simple)
