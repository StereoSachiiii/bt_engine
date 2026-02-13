#pragma once
#include <windows.h>
#include <cstdint>


//the clock seems to be not working properly its innacurate half the time.. you gotta fix this .. IMPORTANT!!
class Timer {
private:
    LARGE_INTEGER frequency_;
    LARGE_INTEGER start_;

public:
    Timer() {
        //this hook stores the number of ticks per second ns etc into the arg
        QueryPerformanceFrequency(&frequency_);
    }

    void start() {
        //marks the begninning
        //this hook stores the current tick number of the system
        QueryPerformanceCounter(&start_);
    }

    LARGE_INTEGER get_start() {
        return start_;
    }
    
    LARGE_INTEGER get_freq() {
        return frequency_;
    }

    uint64_t elapsed_ns() const {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        // get number of sticks from start() to calling elapsed_ns()
        uint64_t elapsed = end.QuadPart - start_.QuadPart;
        return static_cast<uint64_t>(elapsed) * 1'000'000'000ULL / frequency_.QuadPart;
    }

    uint64_t elapsed_us() const {
        return elapsed_ns() / 1000;
    }

    uint64_t elapsed_ms() const {
        return elapsed_ns() / 1'000'000;
    }
};