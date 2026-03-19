cmake_minimum_required(VERSION 3.10)
project(hft_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Core library (header-only for now) ----
add_library(hft_core INTERFACE)
target_include_directories(hft_core INTERFACE ${CMAKE_SOURCE_DIR}/src)

# ---- Timer test executable ----
add_executable(timer_test
    tests/timer_test.cpp
)
target_link_libraries(timer_test PRIVATE hft_core)

# ---- Queue benchmark executable ----
add_executable(queue_benchmark
    benchmarks/queue_benchmark.cpp
)
target_link_libraries(queue_benchmark PRIVATE hft_core)


# ---- Object pool test executable ----
add_executable(object_pool_test
    tests/object_pool_test.cpp
)
target_link_libraries(object_pool_test PRIVATE hft_core)

# ---- ITCH test executable ----
add_executable(test_itch
    tests/test_itch.cpp
)
target_link_libraries(test_itch PRIVATE hft_core)
if(WIN32)
    target_link_libraries(test_itch PRIVATE ws2_32)
endif()
