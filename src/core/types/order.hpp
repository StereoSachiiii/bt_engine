#pragma once
#include <cstdint>

struct Order {
    // intrusive list
    Order* next;
    Order* prev;

    uint64_t order_ref;
    uint64_t timestamp_ns;
    uint64_t price;
    uint64_t new_order_ref;
    uint32_t shares;
    uint16_t stock_locate;
    char     side;
    char     msg_type;
    
    union {
        struct {
            uint8_t  printable;
            char     market_category;
            char     reserved[6];
        };
        char symbol[8];
    };
};

static_assert(sizeof(Order) == 64, "Order must be 64 bytes");
