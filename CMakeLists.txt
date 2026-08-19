cmake_minimum_required(VERSION 3.20)

project(AegisX
    VERSION 0.1.0
    DESCRIPTION "Deterministic event-driven electronic exchange simulator"
    LANGUAGES CXX
)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AEGISX_BUILD_TESTS "Build AegisX tests" ${BUILD_TESTING})

add_library(aegisx_core STATIC
    src/project.cpp
)

target_include_directories(aegisx_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(aegisx_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(aegisx_core PRIVATE /W4 /permissive- /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(aegisx_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()

add_executable(aegisx examples/demo.cpp)
target_link_libraries(aegisx PRIVATE aegisx_core)

if(AEGISX_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(aegisx_tests tests/smoke_test.cpp)
    target_link_libraries(aegisx_tests PRIVATE aegisx_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(aegisx_tests)
endif()
