set(TESTS
    reflect.cpp
    introspect.cpp
    proxy.cpp
    json.cpp
    string_literal.cpp
)

foreach(file ${TESTS})
	get_filename_component(name ${file} NAME_WLE)
    add_executable(${name}_test ${file})
    target_link_libraries(${name}_test PRIVATE Catch2::Catch2WithMain tsmp::json)
    enable_reflection(${name}_test)
    catch_discover_tests(${name}_test)
    target_compile_options(${name}_test PRIVATE ${TSMP_CMAKE_CXX_FLAGS})
endforeach()

add_subdirectory(tooling)