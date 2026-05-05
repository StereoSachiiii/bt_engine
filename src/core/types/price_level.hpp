#pragma once
#include <cstdint>
#include <cstddef>

struct Order;

struct PriceLevel {
    uint64_t price;
    uint64_t total_qty;
    Order* head;      // first in queue (fills first)
    Order* tail;      // append new orders here
    uint32_t order_count;
};
