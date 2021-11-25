find_package(fmt REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(simple ${CMAKE_CURRENT_LIST_DIR}/simple.cpp)
target_link_libraries(simple PUBLIC fmt::fmt)
enable_reflection(simple)

add_executable(json ${CMAKE_CURRENT_LIST_DIR}/json.cpp)
target_link_libraries(json PUBLIC fmt::fmt)
enable_reflection(json)