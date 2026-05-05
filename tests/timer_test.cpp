#include "core/timer.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Timer t;

    // Test 1: Measure 10ms sleep
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto elapsed = t.elapsed_ms();

    std::cout << "10ms sleep measured as: " << elapsed << "ms\n";
    
    // On Windows, sleep_for can be slightly less than requested or more depending on timer resolution
    if (elapsed >= 9 && elapsed <= 20) {
        std::cout << "✓ Timer working (within expected tolerance)\n";
    }
    else {
        std::cout << "✗ Timer broken (elapsed: " << elapsed << "ms)\n";
    }


    return 0;
}