set(DEPENDENCY_LOADING "FetchContent" CACHE STRING "Choose how dependencies should be resolved.")
set_property(CACHE DEPENDENCY_LOADING PROPERTY STRINGS System FetchContent Vcpkg)
if(${DEPENDENCY_LOADING} STREQUAL "Vcpkg")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg installation toolchain")
    if(NOT EXISTS ${})
        message(FATAL "Vcpkg submodule is not initialised by git. Please use 'git submodule update --init --recursive' to load it into the source tree.")
    else()
        message(STATUS "Set CMAKE_TOOLCHAIN_FILE to ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

macro(LoadDependencies)
    if(${DEPENDENCY_LOADING} STREQUAL "FetchContent")
        include(FetchContent)
        unset(CMAKE_TOOLCHAIN_FILE CACHE)
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG        9.1.0
        )
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.2
        )
        if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
            FetchContent_Declare(
                tsmpCatch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG        v3.1.0
            )
            FetchContent_MakeAvailable(nlohmann_json tsmpCatch2 fmt)
            list(APPEND CMAKE_MODULE_PATH "${tsmpCatch2_SOURCE_DIR}/extras")
        else()
            FetchContent_MakeAvailable(nlohmann_json fmt)
        endif()
    else()
        find_package(Catch2 CONFIG REQUIRED)
        find_package(nlohmann_json CONFIG REQUIRED)
        find_package(fmt CONFIG REQUIRED)
    endif()
    find_package(LibClangCpp CONFIG REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endmacro()