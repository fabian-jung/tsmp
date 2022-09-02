add_executable(simple ${CMAKE_CURRENT_LIST_DIR}/simple.cpp)
target_link_libraries(simple PUBLIC fmt::fmt)
target_compile_options(simple PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
enable_reflection(simple)

add_executable(json ${CMAKE_CURRENT_LIST_DIR}/json.cpp)
target_link_libraries(json PUBLIC fmt::fmt)
target_compile_options(json PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
enable_reflection(json)

add_executable(json_encode ${CMAKE_CURRENT_LIST_DIR}/json_encode.cpp)
target_link_libraries(json_encode PUBLIC tsmp::json)
target_compile_options(json_encode PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
enable_reflection(json_encode)

add_executable(proxy ${CMAKE_CURRENT_LIST_DIR}/proxy.cpp)
target_link_libraries(proxy PUBLIC fmt::fmt)
target_compile_options(proxy PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
enable_reflection(proxy)

add_executable(enum ${CMAKE_CURRENT_LIST_DIR}/enum.cpp)
target_link_libraries(enum PUBLIC fmt::fmt)
target_compile_options(enum PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
enable_reflection(enum)