#include "../hft-engine/src/core/timer.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Timer t;

    // Test 1: Measure 10ms sleep
    std::cout << "freq : " << static_cast<uint64_t>(t.get_freq().QuadPart) << std::endl;

    t.start();
    //dispaly start
    std::cout << "start : " << static_cast<uint64_t>(t.get_start().QuadPart)  << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto elapsed = t.elapsed_ms();

    std::cout << "10ms sleep measured as: " << elapsed << "ms\n";
    std::cout << "Error: " << (elapsed - 10) << "ms\n";

    if (elapsed >= 9 && elapsed <= 12) {
        std::cout << "✓ Timer working (within 20% tolerance)\n";
    }
    else {
        std::cout << "✗ Timer broken\n";
    }

    return 0;
}