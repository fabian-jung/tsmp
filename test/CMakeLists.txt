set(TESTS
    ${CMAKE_CURRENT_LIST_DIR}/reflect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/introspect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxy.cpp
)

foreach(file ${TESTS})
	get_filename_component(name ${file} NAME_WLE)
    add_executable(${name}_test ${file})
    target_link_libraries(${name}_test PRIVATE Catch2::Catch2WithMain tsmp)
    enable_reflection(${name}_test)
    catch_discover_tests(${name}_test)
endforeach()

add_executable(reflect_without_linking_test reflect_without_linking.cpp)
target_link_libraries(reflect_without_linking_test PRIVATE Catch2::Catch2WithMain tsmp)
catch_discover_tests(reflect_without_linking_test)


add_subdirectory(tooling)