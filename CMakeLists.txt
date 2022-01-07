project(introspect)
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(Catch2 REQUIRED)
include(CTest NO_POLICY_SCOPE)
include(Catch NO_POLICY_SCOPE)
include(ParseAndAddCatchTests)

add_subdirectory(tooling)

add_library(tsmp INTERFACE)
target_include_directories(tsmp INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(tsmp INTERFACE cxx_std_20)

add_subdirectory(examples)

add_subdirectory(test)

install(TARGETS tsmp introspect_tool EXPORT tsmpConfig DESTINATION lib/tsmp)
install(EXPORT tsmpConfig NAMESPACE tsmp:: FILE tsmpTargets.cmake DESTINATION lib/cmake/tsmp EXPORT_LINK_INTERFACE_LIBRARIES)

install(TARGETS introspect_tool RUNTIME DESTINATION bin)
install(FILES tsmpConfig.cmake DESTINATION lib/cmake/tsmp)
install(DIRECTORY include/tsmp DESTINATION include FILES_MATCHING PATTERN "*.h*")