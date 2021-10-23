project(introspect)
cmake_minimum_required(VERSION 3.0.0)

add_subdirectory(bin)

add_library(tsmp INTERFACE)
target_include_directories(tsmp INTERFACE include)
target_compile_features(tsmp INTERFACE cxx_std_20)

add_subdirectory(examples)