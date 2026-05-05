#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>


struct alignas(64) OrderBookEvent {
    uint64_t timestamp_ns;     // 8 bytes
    uint64_t order_ref;        // 8 bytes
    double price;              // 8 bytes
    uint32_t filled_shares;    // 4 bytes
    uint32_t remaining_shares; // 4 bytes
    uint8_t event_type;        // 1 byte
    char side;                 // 1 byte (B/S)
    
    char reserved[30];         // Pad to exactly 64 bytes
};

static_assert(sizeof(OrderBookEvent) == 64, "OrderBookEvent must be 64 bytes");
static_assert(alignof(OrderBookEvent) == 64, "OrderBookEvent must be cache-line aligned");
