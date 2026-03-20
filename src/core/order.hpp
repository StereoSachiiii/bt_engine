#pragma once
#include <cstdint>

struct Order {
    uint64_t order_ref;
    uint64_t timestamp_ns;
    double   price;
    uint32_t shares;
    char     stock[8];
    char     side;
    char     msg_type;
    char     reserved[30];

    
    Order()
        : order_ref(0),
        timestamp_ns(0),
        price(0.0),
        shares(0),
        stock{},
        side(0),
        msg_type(0),
        reserved{}
    {
    }
};

static_assert(sizeof(Order) == 64, "Order must be 64 bytes");