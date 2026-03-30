#include <iostream>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cstring>

#include "src/core/data/itch_file_parser.hpp"  // your parser

// Black box to prevent compiler optimization
volatile int sink;
void DoNotOptimize(const Order& order) {
    sink += *reinterpret_cast<const volatile int*>(&order);
}

int main() {
    ITCHParser::init();

    // Fake ITCH message (Add Order)
    alignas(64) uint8_t msg[40] = { 0 };

    msg[0] = 'A';

    // Fill with some realistic data
    for (int i = 0; i < 40; i++) {
        msg[i] = uint8_t(i);
    }

    Order order;

    const size_t iterations = 1'000'000;  // Reduced to 1M for troubleshooting

    // Warm-up (important!)
    for (size_t i = 0; i < 10'000; i++) {
        ITCHParser::parse(msg, order);
    }

    // Single measurement to verify
    std::cout << "Order size: " << sizeof(Order) << " bytes\n";

    // Start timing using steady_clock for better precision
    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < iterations; i++) {
        ITCHParser::parse(msg, order);
        DoNotOptimize(order);  // Call every iteration
    }

    auto end = std::chrono::steady_clock::now();

    // Compute results
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    double ns_per_msg = static_cast<double>(duration_ns) / static_cast<double>(iterations);
    double msgs_per_sec = 1e9 / ns_per_msg;

    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total time: " << duration_ns << " ns (" << (duration_ns / 1e6) << " ms)\n";
    std::cout << "Per message: " << ns_per_msg << " ns\n";
    std::cout << "Throughput: " << (msgs_per_sec / 1e6) << " M msgs/sec\n";

    // Print sink to force it to be evaluated
    volatile int printed_sink = sink;
    std::cout << "Sink (used to prevent optimization): " << printed_sink << "\n";

    return 0;
}