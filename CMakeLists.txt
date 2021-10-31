project(introspect)
cmake_minimum_required(VERSION 3.0.0)

add_subdirectory(tooling)

add_library(tsmp INTERFACE)
target_include_directories(tsmp INTERFACE include)
target_compile_features(tsmp INTERFACE cxx_std_20)

function(enable_reflection target)
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    file(RELATIVE_PATH RELATIVE_SOURCES ${CMAKE_BINARY_DIR} ${TARGET_SOURCES})
    message(STATUS "Sources for ${target}: ${TARGET_SOURCES} ${RELATIVE_SOURCES}")
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
        COMMAND ./bin/introspect_tool ${RELATIVE_SOURCES} ./tsmp/${target}/reflection.hpp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS introspect_tool
        DEPENDS ${TARGET_SOURCES}
    )
    add_custom_target(${target}_header
        DEPENDS ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
    )
    target_link_libraries(${target} PUBLIC tsmp)
    add_dependencies(${target} ${target}_header)
    target_include_directories(${target} PUBLIC ${CMAKE_BINARY_DIR}/tsmp/${target})
endfunction()

add_subdirectory(examples)

find_package(Catch2 REQUIRED)
include(CTest NO_POLICY_SCOPE)
include(Catch NO_POLICY_SCOPE)
include(ParseAndAddCatchTests)
add_subdirectory(test)