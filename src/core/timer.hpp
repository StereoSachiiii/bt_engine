#pragma once
#include <chrono>
#include <cstdint>


class Timer {
private:
    std::chrono::steady_clock::time_point start_;

public:
    Timer() {
        start();
    }


    void start() {
        start_ = std::chrono::steady_clock::now();
    }


    uint64_t elapsed_ns() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
    }


    uint64_t elapsed_us() const {
        return elapsed_ns() / 1000;
    }


    uint64_t elapsed_ms() const {
        return elapsed_ns() / 1000000;
    }
};
