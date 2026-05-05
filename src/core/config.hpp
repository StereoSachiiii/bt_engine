#pragma once
#include <cstddef>

namespace config {
    // Pool sizing for S&P 500 universe (~500 symbols)
    // Peak live orders: ~2K-8K per symbol, 4M gives headroom for vol spikes
    inline constexpr size_t MAX_ORDERS       = 4'000'000;
    inline constexpr size_t MAX_PRICE_LEVELS = 500'000;

    // Order book geometry
    inline constexpr size_t PRICE_WINDOW     = 1'000'000;
    inline constexpr size_t INDEX_SIZE        = 1 << 20;  // 1M slots, ~25% load factor at 4M orders (with chaining)
    inline constexpr size_t INDEX_MASK        = INDEX_SIZE - 1;
}
