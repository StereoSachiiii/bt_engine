#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>


struct OrderBookEvent {
    uint64_t order_ref;
    double price;
    uint32_t filled_shares;
    uint32_t remaining_shares;
    char side;  // 'B' or 'S'
    uint8_t event_type;  // ADD, FILL, CANCEL, etc.
    uint64_t timestamp_ns;
    char reserved[23];  // Pad to 64 bytes for cache alignment
};

static_assert(sizeof(OrderBookEvent) == 64, "OrderBookEvent must be 64 bytes");