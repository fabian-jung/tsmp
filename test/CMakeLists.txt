find_package(Catch2 REQUIRED)
include(CTest NO_POLICY_SCOPE)
include(Catch NO_POLICY_SCOPE)
include(ParseAndAddCatchTests)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

set(TESTS
    ${CMAKE_CURRENT_LIST_DIR}/reflect.cpp
)

foreach(file ${TESTS})
	get_filename_component(name ${file} NAME_WLE)
    add_executable(${name}_test ${file})
    target_link_libraries(${name}_test PRIVATE catch_main tsmp)
    enable_reflection(${name}_test)
    catch_discover_tests(${name}_test)
endforeach()