cmake_minimum_required(VERSION 3.10)
project(hft_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_ASAN "Enable AddressSanitizer" OFF)
if(USE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        # MSVC ASan requires the library to be linked
        # No explicit link option needed usually, but /INCREMENTAL:NO is recommended
        add_link_options(/INCREMENTAL:NO)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
    message(STATUS "AddressSanitizer enabled")
endif()

# ---- Core library (header-only for now) ----
add_library(hft_core INTERFACE)
target_include_directories(hft_core INTERFACE ${CMAKE_SOURCE_DIR}/src)
# Also add project root for relative includes like ../src/... in subdirectories
target_include_directories(hft_core INTERFACE ${CMAKE_SOURCE_DIR})
target_compile_definitions(hft_core INTERFACE _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)

# ---- Timer test executable ----
add_executable(timer_test
    tests/timer_test.cpp)
target_link_libraries(timer_test PRIVATE hft_core)

# ---- Queue test executable ----
add_executable(queue_test
    tests/queue_test.cpp)
target_link_libraries(queue_test PRIVATE hft_core)

# ---- SPMC Queue test executable ----
add_executable(spmc_queue_test
    tests/spmc_queue_test.cpp)
target_link_libraries(spmc_queue_test PRIVATE hft_core)

# ---- Queue benchmark executable ----
add_executable(queue_benchmark
    benchmarks/queue_benchmark.cpp)
target_link_libraries(queue_benchmark PRIVATE hft_core)

# ---- Object pool test executable ----
add_executable(object_pool_test
    tests/object_pool_test.cpp)
target_link_libraries(object_pool_test PRIVATE hft_core)

# ---- ITCH test executable ----
add_executable(test_itch
    tests/test_itch.cpp)
target_link_libraries(test_itch PRIVATE hft_core)
if(WIN32)
    target_link_libraries(test_itch PRIVATE ws2_32)
endif()

# ---- Window test executable (Windows only) ----
if(WIN32)
    add_executable(window_test
        tests/window_test.cpp)
    target_link_libraries(window_test PRIVATE user32)
endif()


add_executable(itch_parser_test
    tests/itch_parser_test.cpp)
target_link_libraries(itch_parser_test PRIVATE hft_core)

# ---- Realistic ITCH benchmark executable ----
add_executable(realistic_itch_benchmark
    benchmarks/realistic_itch_benchmark.cpp)
target_link_libraries(realistic_itch_benchmark PRIVATE hft_core)

# ---- OrderBook test executable ----
add_executable(orderbook_test
    tests/orderbook_test.cpp)
target_link_libraries(orderbook_test PRIVATE hft_core)

# ---- Hierarchical Bitset test executable ----
add_executable(bitset_test
    tests/bitset_test.cpp)
target_link_libraries(bitset_test PRIVATE hft_core)

# ---- Main Engine executable ----
add_executable(hft_engine
    src/main.cpp)
target_link_libraries(hft_engine PRIVATE hft_core)

# ---- Strategy test ----
add_executable(strategy_test
    tests/strategy_test.cpp)
target_link_libraries(strategy_test PRIVATE hft_core)

# ---- Full system benchmark ----
add_executable(full_system_benchmark
    benchmarks/full_system_benchmark.cpp)
target_link_libraries(full_system_benchmark PRIVATE hft_core)
