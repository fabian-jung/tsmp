set(TESTS
    ${CMAKE_CURRENT_LIST_DIR}/prefix_splitter.cpp
)

foreach(file ${TESTS})
	get_filename_component(name ${file} NAME_WLE)
    add_executable(${name}_test ${file})
    target_link_libraries(${name}_test PRIVATE catch_main introspect_lib)
    enable_reflection(${name}_test)
    catch_discover_tests(${name}_test)
endforeach()