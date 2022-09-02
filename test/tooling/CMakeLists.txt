set(TESTS
    prefix_splitter.cpp
)

foreach(file ${TESTS})
	get_filename_component(name ${file} NAME_WLE)
    add_executable(${name}_test ${file})
    target_link_libraries(${name}_test PRIVATE Catch2::Catch2WithMain introspect_lib)
    enable_reflection(${name}_test)
    catch_discover_tests(${name}_test)
    target_compile_options(${name}_test PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
endforeach()