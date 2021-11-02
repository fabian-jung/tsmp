add_library(llvm_config INTERFACE)
find_library(libclangcpp "libclang-cpp.so" PATHS "/usr/lib/llvm-12/lib")
find_library(libclang "libclang.so" PATHS "/usr/lib/llvm-12/lib")
find_library(libllvm "libLLVM.so" PATHS "/usr/lib/llvm-12/lib")
target_include_directories(llvm_config INTERFACE /usr/lib/llvm-12/include)
target_link_libraries(llvm_config INTERFACE ${libclangcpp} ${libclang} ${libllvm})
target_compile_features(llvm_config INTERFACE cxx_std_17)

find_package(fmt REQUIRED)

set(introspect_lib_src
    data/aggregator.cpp
    data/types.cpp
    data/prefix_splitter.cpp
    engine/introspect_visitor.cpp
    engine/utils.cpp
    engine/consumer.cpp
    engine/frontendaction.cpp
)

add_library(introspect_lib STATIC ${introspect_lib_src})
target_link_libraries(introspect_lib PUBLIC llvm_config fmt::fmt)
target_compile_features(introspect_lib PUBLIC cxx_std_17)
target_include_directories(introspect_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(introspect_tool bin/introspect.cpp)
target_link_libraries(introspect_tool PUBLIC introspect_lib)

function(enable_reflection target)
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    file(RELATIVE_PATH RELATIVE_SOURCES ${CMAKE_BINARY_DIR} ${TARGET_SOURCES})
    message(STATUS "Sources for ${target}: ${TARGET_SOURCES} ${RELATIVE_SOURCES}")
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
        COMMAND ./tooling/introspect_tool ${RELATIVE_SOURCES} ./tsmp/${target}/reflection.hpp
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
