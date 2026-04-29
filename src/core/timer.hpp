#pragma once
#include <chrono>
#include <cstdint>

/**
 * @brief High-resolution monotonic timer for performance measurement.
 * Uses std::chrono::steady_clock for cross-platform accuracy and nanosecond precision.
 */
class Timer {
private:
    std::chrono::steady_clock::time_point start_;

public:
    Timer() {
        start();
    }

    /**
     * @brief Resets the timer to the current time point.
     */
    void start() {
        start_ = std::chrono::steady_clock::now();
    }

    /**
     * @return Elapsed time in nanoseconds.
     */
    uint64_t elapsed_ns() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
    }

    /**
     * @return Elapsed time in microseconds.
     */
    uint64_t elapsed_us() const {
        return elapsed_ns() / 1000;
    }

    /**
     * @return Elapsed time in milliseconds.
     */
    uint64_t elapsed_ms() const {
        return elapsed_ns() / 1000000;
    }
};