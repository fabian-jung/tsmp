cmake_minimum_required(VERSION 3.12.0)
include(cmake/dependencyLoading.cmake)
project(tsmp
    VERSION 1.0.0
    DESCRIPTION "Library for intrusion free, compile-time reflection with the help of code generation."
    LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE CACHE INTERNAL "The compile database is used by the tsmp introspecter and must be generated")
LoadDependencies()

if(MSVC)
    SET(TSMP_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /W4 /WX)
else()
    SET(TSMP_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-missing-braces)
endif()

add_subdirectory(tooling)

add_library(tsmp INTERFACE)
target_include_directories(tsmp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(tsmp INTERFACE cxx_std_20)
add_dependencies(tsmp INTERFACE
    include/introspect.hpp
    include/proxy.hpp
    include/reflect.hpp
    include/string_literal.hpp
)

add_library(tsmp_json INTERFACE)
add_library(tsmp::json ALIAS tsmp_json)
target_link_libraries(tsmp_json INTERFACE fmt::fmt nlohmann_json::nlohmann_json)
add_dependencies(tsmp INTERFACE
    include/tsmp.hpp
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # if CMAKE_CURRENT_SOURCE_DIR != CMAKE_SOURCE_DIR, tsmp has been loaded via add_subdirectory or FetchContent
    # in this case we do not to pollute the global configuration with the example and test targets.
    add_subdirectory(examples)
    add_subdirectory(test)

    install(TARGETS tsmp introspect_tool EXPORT tsmpConfig DESTINATION lib/tsmp)
    install(EXPORT tsmpConfig NAMESPACE tsmp:: FILE tsmpTargets.cmake DESTINATION lib/cmake/tsmp)

    install(TARGETS introspect_tool RUNTIME DESTINATION bin)
    install(FILES cmake/tsmpConfig.cmake DESTINATION lib/cmake/tsmp)
    install(DIRECTORY include/tsmp DESTINATION include FILES_MATCHING PATTERN "*.h*")
endif()