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
option(AEGISX_BUILD_BENCHMARKS "Build AegisX benchmarks" ON)

add_library(aegisx_core STATIC
    src/order.cpp
    src/order_book.cpp
    src/price_level.cpp
    src/project.cpp
    src/replay.cpp
    src/reference_engine.cpp
    src/matching_engine.cpp
    src/exchange.cpp
    src/invariants.cpp
    src/simulation.cpp
    src/trade.cpp
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

add_executable(aegisx-cli tools/cli.cpp)
target_link_libraries(aegisx-cli PRIVATE aegisx_core)

add_executable(aegisx-sim tools/simulator.cpp)
target_link_libraries(aegisx-sim PRIVATE aegisx_core)

add_executable(aegisx-replay tools/replay.cpp)
target_link_libraries(aegisx-replay PRIVATE aegisx_core)

if(AEGISX_BUILD_BENCHMARKS)
    FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googlebenchmark)

    add_executable(aegisx_benchmarks benchmarks/benchmark_suite.cpp)
    target_link_libraries(aegisx_benchmarks PRIVATE aegisx_core benchmark::benchmark_main)
endif()

if(AEGISX_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(aegisx_tests
        tests/order_tests.cpp
        tests/order_book_tests.cpp
        tests/event_tests.cpp
        tests/exchange_tests.cpp
        tests/invariant_tests.cpp
        tests/simulation_tests.cpp
        tests/replay_tests.cpp
        tests/reference_tests.cpp
        tests/cancellation_tests.cpp
        tests/matching_tests.cpp
        tests/modification_tests.cpp
        tests/price_level_tests.cpp
        tests/smoke_test.cpp
        tests/trade_tests.cpp
    )
    target_link_libraries(aegisx_tests PRIVATE aegisx_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(aegisx_tests)
endif()
