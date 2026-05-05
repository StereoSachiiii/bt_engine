#include <iostream>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cstring>

#include "src/core/data/itch_file_parser.hpp"  //  parser

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
    msg[1] = 0x01; msg[2] = 0x23; // stock_locate = 0x0123 = 291

    // Fill with some realistic data
    for (int i = 3; i < 40; i++) {
        msg[i] = uint8_t(i);
    }

    Order order;
    ITCHParser::parse(msg, order);

    if (order.msg_type != 'A' || order.stock_locate != 291) {
        std::cerr << "FAILED: Add Order (A) parsing incorrect\n";
        return 1;
    }

    // Test Executed (E)
    msg[0] = 'E';
    msg[19] = 0x00; msg[20] = 0x00; msg[21] = 0x03; msg[22] = 0xE8; // Shares = 1000
    ITCHParser::parse(msg, order);
    if (order.msg_type != 'E' || order.shares != 1000) {
        std::cerr << "FAILED: Executed (E) parsing incorrect\n";
        return 1;
    }

    // Test Cancel (X)
    msg[0] = 'X';
    msg[19] = 0x00; msg[20] = 0x00; msg[21] = 0x01; msg[22] = 0xF4; // Shares = 500
    ITCHParser::parse(msg, order);
    if (order.msg_type != 'X' || order.shares != 500) {
        std::cerr << "FAILED: Cancel (X) parsing incorrect\n";
        return 1;
    }

    // Test Replace (U)
    msg[0] = 'U';
    msg[11] = 0x00; msg[12] = 0x00; msg[13] = 0x00; msg[14] = 0x00; msg[15] = 0x00; msg[16] = 0x00; msg[17] = 0x04; msg[18] = 0xD2; // Old Ref = 1234
    msg[19] = 0x00; msg[20] = 0x00; msg[21] = 0x00; msg[22] = 0x00; msg[23] = 0x00; msg[24] = 0x00; msg[25] = 0x16; msg[26] = 0x2E; // New Ref = 5678
    msg[27] = 0x00; msg[28] = 0x00; msg[29] = 0x00; msg[30] = 0x64; // New Shares = 100
    msg[31] = 0x00; msg[32] = 0x00; msg[33] = 0x3A; msg[34] = 0x98; // New Price = 15000
    ITCHParser::parse(msg, order);
    if (order.msg_type != 'U' || order.order_ref != 1234 || order.new_order_ref != 5678 || order.shares != 100 || order.price != 15000) {
        std::cerr << "FAILED: Replace (U) parsing incorrect\n";
        return 1;
    }

    std::cout << "SUCCESS: All message types verified (A, E, X, U)\n";

    const size_t iterations = 1'000'000; 

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