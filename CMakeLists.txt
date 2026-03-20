cmake_minimum_required(VERSION 3.10)
project(hft_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Core library (header-only for now) ----
add_library(hft_core INTERFACE)
target_include_directories(hft_core INTERFACE ${CMAKE_SOURCE_DIR}/src)
# Also add project root for relative includes like ../src/... in subdirectories
target_include_directories(hft_core INTERFACE ${CMAKE_SOURCE_DIR})
target_compile_definitions(hft_core INTERFACE _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)

# ---- Timer test executable ----
add_executable(timer_test
    tests/timer_test.cpp
)
target_link_libraries(timer_test PRIVATE hft_core)

# ---- Queue test executable ----
add_executable(queue_test
    tests/queue_test.cpp
)
target_link_libraries(queue_test PRIVATE hft_core)

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

# ---- Window test executable (Windows only) ----
if(WIN32)
    add_executable(window_test
        tests/window_test.cpp
    )
    target_link_libraries(window_test PRIVATE user32)
endif()
