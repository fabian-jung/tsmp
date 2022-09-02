find_library(libclangcpp "libclang-cpp.so" PATHS "/usr/lib/" "/usr/lib/llvm-14/lib")
message(STATUS ${libclangcpp})
if(NOT libclangcpp)
    message(FATAL_ERROR "libclang-cpp could not be found. Make sure it is installed on your sytem")
endif()

find_library(libllvm "libLLVM.so" PATHS "/usr/lib/" "/usr/lib/llvm-14/lib")
if(NOT libllvm)
    message(FATAL_ERROR "libLLVM could not be found. Make sure it is installed on your sytem")
endif()

add_library(LibClangCpp INTERFACE)
add_library(LibClangCpp::LibClangCpp ALIAS LibClangCpp)
target_include_directories(LibClangCpp INTERFACE /usr/lib/llvm-14/include)

target_link_libraries(LibClangCpp INTERFACE ${libllvm} ${libclangcpp})
target_compile_features(LibClangCpp INTERFACE cxx_std_17)