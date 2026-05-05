#pragma once
#include <cstdint>
#include <cstring>

#include "utils/compiler.hpp"

/**
 * @brief 4-Layer Hierarchical Bitset for O(log64 N) price level tracking.
 * best bid/ask takes exactly 4 memory lookups,
 * regardless of the price distribution .
 * L0: 1,000,000 bits (15,625 x 64-bit words)  one bit per price level
 * L1: 245 bits (4 x 64-bit words)  summary of L0
 * L2: 4 bits (1 x 64-bit word)  summary of L1
 * L3: 1 x 64-bit word summary of L2
 */
struct HierarchicalBitset {
    static constexpr size_t L0_SIZE = 1000000;
    static constexpr size_t L0_WORDS = (L0_SIZE + 63) / 64;   // 15,625 words
    static constexpr size_t L1_WORDS = (L0_WORDS + 63) / 64;  // 245 words
    static constexpr size_t L2_WORDS = (L1_WORDS + 63) / 64;  // 4 words
    
    uint64_t l0[L0_WORDS];
    uint64_t l1[L1_WORDS];
    uint64_t l2[L2_WORDS];
    uint64_t l3; 

    HierarchicalBitset() : l3(0) {
        memset(l0, 0, sizeof(l0));
        memset(l1, 0, sizeof(l1));
        memset(l2, 0, sizeof(l2));
    }

  
    FORCE_INLINE void set(size_t i) {
        size_t i0 = i >> 6;
        size_t i1 = i0 >> 6;
        size_t i2 = i1 >> 6;
        
        l0[i0] |= (1ULL << (i & 63));
        l1[i1] |= (1ULL << (i0 & 63));
        l2[i2] |= (1ULL << (i1 & 63));
        l3     |= (1ULL << (i2 & 63));
    }


    FORCE_INLINE void reset(size_t i) {
        size_t i0 = i >> 6;
        l0[i0] &= ~(1ULL << (i & 63));
        if (l0[i0] != 0) return; 
        
        size_t i1 = i0 >> 6;
        l1[i1] &= ~(1ULL << (i0 & 63));
        if (l1[i1] != 0) return;
        
        size_t i2 = i1 >> 6;
        l2[i2] &= ~(1ULL << (i1 & 63));
        if (l2[i2] != 0) return;
        
        l3 &= ~(1ULL << (i2 & 63));
    }

    /**
     * Find the lowest set bit index (best ask).
     * Time complexity O(1)  4 lookups + bit operation through hardware instruction
     * @return -1 if empty
     */
    FORCE_INLINE int find_first() const {
        if (l3 == 0) [[unlikely]] return -1;
        size_t i2 = __builtin_ctzll(l3);
        size_t i1 = (i2 << 6) + __builtin_ctzll(l2[i2]);
        size_t i0 = (i1 << 6) + __builtin_ctzll(l1[i1]);
        return static_cast<int>((i0 << 6) + __builtin_ctzll(l0[i0]));
    }

    /**
     * Find the highest set bit index (best bid).
     * Time complexity O(1)  4 lookups + bit operation through hardware instruction
     * @return -1 if empty
     */
    FORCE_INLINE int find_last() const {
        if (l3 == 0) [[unlikely]] return -1;
        size_t i2 = 63 - __builtin_clzll(l3);
        size_t i1 = (i2 << 6) + (63 - __builtin_clzll(l2[i2]));
        size_t i0 = (i1 << 6) + (63 - __builtin_clzll(l1[i1]));
        return static_cast<int>((i0 << 6) + (63 - __builtin_clzll(l0[i0])));
    }

    
    FORCE_INLINE bool is_empty() const {
        return l3 == 0;
    }
};
