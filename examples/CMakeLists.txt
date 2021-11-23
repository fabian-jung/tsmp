add_library(simple_cpp_header INTERFACE)
add_dependencies(simple_cpp_header simple.cpp.header)
target_include_directories(simple_cpp_header INTERFACE ${CMAKE_BINARY_DIR}/examples/simple.cpp.dir)

find_package(fmt REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
add_executable(simple ${CMAKE_CURRENT_LIST_DIR}/simple.cpp)
target_link_libraries(simple PUBLIC fmt::fmt)
enable_reflection(simple)
