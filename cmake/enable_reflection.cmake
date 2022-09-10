function(prefix_paths OUTPUT INPUT)
    foreach(FILE ${INPUT})
        if(NOT IS_ABSOLUTE ${FILE})
        set(FILE ${CMAKE_CURRENT_LIST_DIR}/${FILE})
        endif()
        file(RELATIVE_PATH RELATIVE_SOURCE ${CMAKE_BINARY_DIR} ${FILE})
        list(APPEND RESULT ${RELATIVE_SOURCE})
    endforeach()
    set(${OUTPUT} ${RESULT} PARENT_SCOPE)
endfunction()

function(enable_reflection target)
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    prefix_paths(ABSOLUTE_SOURCES "${TARGET_SOURCES}")

    message(STATUS "Sources for ${target} with sources: ${ABSOLUTE_SOURCES}")
    add_custom_target(
        ${target}_header_dir
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tsmp/${target}
    )

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tsmp/${target}/builtInInclude.path
        COMMAND ${CMAKE_CXX_COMPILER} -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n "s,^ ,,p" > ${CMAKE_BINARY_DIR}/tsmp/${target}/builtInInclude.path
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${target}_header_dir
        VERBATIM
    )
    add_custom_target(
        ${target}_builtin_includes
        DEPENDS ${CMAKE_BINARY_DIR}/tsmp/${target}/builtInInclude.path
    )

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
        COMMAND introspect_tool ${ABSOLUTE_SOURCES} ./tsmp/${target}/reflection.hpp 2> ./tsmp/${target}/error.log 1> ./tsmp/${target}/build.log
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS introspect_tool
        DEPENDS "${TARGET_SOURCES}"
        DEPENDS ${target}_builtin_includes
        VERBATIM
    )
    add_custom_target(${target}_header
        DEPENDS ${CMAKE_BINARY_DIR}/tsmp/${target}/reflection.hpp
    )
    target_link_libraries(${target} PUBLIC tsmp)
    add_dependencies(${target} ${target}_header)
    target_include_directories(${target} PUBLIC ${CMAKE_BINARY_DIR}/tsmp/${target})
    target_compile_definitions(${target} PRIVATE TSMP_REFLECTION_ENABLED)
endfunction()