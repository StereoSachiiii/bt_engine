#include "core/hierarchical_bitset.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <random>

void test_bitset_basic() {
    std::cout << "Starting test_bitset_basic...\n";
    HierarchicalBitset hb;
    assert(hb.is_empty());
    assert(hb.find_first() == -1);
    assert(hb.find_last() == -1);

    hb.set(100);
    assert(!hb.is_empty());
    assert(hb.find_first() == 100);
    assert(hb.find_last() == 100);

    hb.set(200);
    assert(hb.find_first() == 100);
    assert(hb.find_last() == 200);

    hb.reset(100);
    assert(hb.find_first() == 200);
    assert(hb.find_last() == 200);

    hb.reset(200);
    assert(hb.is_empty());
    std::cout << "test_bitset_basic OK\n";
}

void test_bitset_boundaries() {
    std::cout << "Starting test_bitset_boundaries...\n";
    HierarchicalBitset hb;
    
    hb.set(0);
    assert(hb.find_first() == 0);
    assert(hb.find_last() == 0);

    hb.set(999999);
    assert(hb.find_first() == 0);
    assert(hb.find_last() == 999999);

    hb.reset(0);
    assert(hb.find_first() == 999999);
    
    hb.reset(999999);
    assert(hb.is_empty());
    std::cout << "test_bitset_boundaries OK\n";
}

void test_bitset_random() {
    std::cout << "Starting test_bitset_random...\n";
    HierarchicalBitset hb;
    std::vector<int> bits;
    
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 999999);

    for(int i = 0; i < 1000; ++i) {
        int bit = dis(gen);
        hb.set(bit);
        bits.push_back(bit);
    }

    std::sort(bits.begin(), bits.end());
    bits.erase(std::unique(bits.begin(), bits.end()), bits.end());

    assert(hb.find_first() == bits.front());
    assert(hb.find_last() == bits.back());

    // Remove half
    for(size_t i = 0; i < bits.size() / 2; ++i) {
        hb.reset(bits[i]);
    }
    
    std::vector<int> remaining(bits.begin() + bits.size() / 2, bits.end());
    assert(hb.find_first() == remaining.front());
    assert(hb.find_last() == remaining.back());

    std::cout << "test_bitset_random OK\n";
}

int main() {
    test_bitset_basic();
    test_bitset_boundaries();
    test_bitset_random();
    std::cout << "All HierarchicalBitset tests passed!\n";
    return 0;
}
