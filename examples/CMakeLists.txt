find_package(fmt REQUIRED)

add_executable(simple ${CMAKE_CURRENT_LIST_DIR}/simple.cpp)
target_link_libraries(simple PUBLIC fmt::fmt)
enable_reflection(simple)

add_executable(json ${CMAKE_CURRENT_LIST_DIR}/json.cpp)
target_link_libraries(json PUBLIC fmt::fmt)
enable_reflection(json)

add_executable(json_encode ${CMAKE_CURRENT_LIST_DIR}/json_encode.cpp)
target_link_libraries(json_encode PUBLIC fmt::fmt nlohmann_json::nlohmann_json)
enable_reflection(json_encode)

add_executable(proxy ${CMAKE_CURRENT_LIST_DIR}/proxy.cpp)
target_link_libraries(proxy PUBLIC fmt::fmt)
enable_reflection(proxy)

add_executable(enum ${CMAKE_CURRENT_LIST_DIR}/enum.cpp)
target_link_libraries(enum PUBLIC fmt::fmt)
enable_reflection(enum)