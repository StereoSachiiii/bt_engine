#include <thread>
#include <vector>
#include <atomic>
#include <cassert>
#include <iostream>
#include "core/object_pool.hpp"

void test_concurrent() {
    constexpr int N = 1024;
    constexpr int THREADS = 8;
    constexpr int OPS = 100000;

    // int (4 bytes) is smaller than void* (8 bytes) on x64,
    // which violates the pool's static_assert. Use int64_t instead.
    ObjectPool<int64_t, N> pool;

    std::atomic<bool> start{ false };
    std::atomic<int> total_alloc{ 0 };

    auto worker = [&]() {
        while (!start.load()) {}

        for (int i = 0; i < OPS; i++) {
            int64_t* p = pool.allocate();

            if (p) {
                total_alloc.fetch_add(1, std::memory_order_relaxed);

                // simulate work
                *p = 42;

                pool.deallocate(p);
            }
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < THREADS; i++) {
        threads.emplace_back(worker);
    }

    start.store(true);

    for (auto& t : threads) {
        t.join();
    }

    // invariant: everything returned
    assert(pool.available() == N);

    std::cout << "concurrent OK\n";
}

int main() {
    test_concurrent();
    return 0;
}