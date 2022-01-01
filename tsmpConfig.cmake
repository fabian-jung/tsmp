set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(CMakeFindDependencyMacro)

# Capturing values from configure (optional)

find_dependency(fmt REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/tsmpTargets.cmake)

function(enable_reflection target)
    set(INTROSPECT_TOOL $<TARGET_FILE:tsmp::introspect_tool>)
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    
    set(RELATIVE_SOURCES "")
    foreach(SRC ${TARGET_SOURCES})
        cmake_path(
            ABSOLUTE_PATH SRC
            BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} 
            NORMALIZE 
            OUTPUT_VARIABLE RSRC
        )
        list(APPEND RELATIVE_SOURCES ${RSRC})
    endforeach()  
    
    message(STATUS "Sources for ${target}: ${TARGET_SOURCES} ${RELATIVE_SOURCES}")
    add_custom_target(
        ${target}_header_dir
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tsmp/${target}
    )
    message(STATUS "Introspect tool used: ${INTROSPECT_TOOL}")
    message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
        COMMAND ${INTROSPECT_TOOL} ${RELATIVE_SOURCES} ./tsmp/${target}/reflection.hpp 2> ./tsmp/${target}/error.log 1> ./tsmp/${target}/build.log
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${TARGET_SOURCES}
        DEPENDS ${target}_header_dir
        VERBATIM
        BYPRODUCTS 
            ./tsmp/${target}/error.log 
            ./tsmp/${target}/build.log
    )
    add_custom_target(${target}_header
        DEPENDS ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
        DEPENDS tsmp::introspect_tool
    )
    target_link_libraries(${target} PUBLIC tsmp::tsmp)
    add_dependencies(${target} ${target}_header)
    target_include_directories(${target} PUBLIC ${CMAKE_BINARY_DIR}/tsmp/${target})
    target_compile_definitions(${target} PRIVATE TSMP_REFLECTION_ENABLED)
endfunction()