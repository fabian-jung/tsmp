add_library(llvm_config INTERFACE)
find_library(libclangcpp "libclang-cpp.so" PATHS "/usr/lib/llvm-12/lib")
find_library(libclang "libclang.so" PATHS "/usr/lib/llvm-12/lib")
find_library(libllvm "libLLVM.so" PATHS "/usr/lib/llvm-12/lib")
target_include_directories(llvm_config INTERFACE /usr/lib/llvm-12/include)
target_link_libraries(llvm_config INTERFACE ${libclangcpp} ${libclang} ${libllvm})
target_compile_features(llvm_config INTERFACE cxx_std_17)

set(introspect_src
    introspect.cpp
    data/aggregator.cpp
    data/types.cpp
    engine/introspect_visitor.cpp
    engine/utils.cpp
    engine/consumer.cpp
    engine/frontendaction.cpp
)

add_executable(introspect_tool ${introspect_src})
target_link_libraries(introspect_tool PUBLIC llvm_config)
target_compile_features(introspect_tool INTERFACE cxx_std_20)
target_include_directories(introspect_tool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})