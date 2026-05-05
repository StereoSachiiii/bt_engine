#include "core/queue.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <numeric>
#include <atomic>

void test_spmc_basic() {
    SPMCQueue<int, 8> q;
    assert(q.try_push(std::move(42)));
    [[maybe_unused]] int out = 0;
    assert(q.try_pop(out));
    assert(out == 42);
    std::cout << "test_spmc_basic OK\n";
}

void test_spmc_multi_consumer() {
    const int num_items = 100000;
    const int num_consumers = 4;
    SPMCQueue<int, 1024> q;
    std::atomic<int> sum{0};
    std::atomic<int> count{0};

    auto consumer = [&]() {
        int val;
        while (count.load(std::memory_order_relaxed) < num_items) {
            if (q.try_pop(val)) {
                sum.fetch_add(val, std::memory_order_relaxed);
                count.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    };

    std::vector<std::thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer);
    }

    for (int i = 1; i <= num_items; ++i) {
        while (!q.try_push(std::move(i))) {
            std::this_thread::yield();
        }
    }

    for (auto& t : consumers) {
        t.join();
    }

    // Expected sum of 1 to num_items: n(n+1)/2
    [[maybe_unused]] long long expected_sum = (long long)num_items * (num_items + 1) / 2;
    assert(sum.load() == expected_sum);
    assert(count.load() == num_items);

    std::cout << "test_spmc_multi_consumer OK (" << num_items << " items, " << num_consumers << " consumers)\n";
}

int main() {
    test_spmc_basic();
    test_spmc_multi_consumer();
    std::cout << "All SPMC tests passed!\n";
    return 0;
}
