#pragma once
#include "../order.hpp"
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include <stdlib.h>
//
////change this this is a heavy bottleneck try to figure out how to use this properly without a bunch of loops
//class ITCHParser {
//
//
//static Order parse_add_order(const uint8_t* data) {
//		Order order;
//
//		
//		//type of the message
//		order.msg_type = data[0];
//
//		// timestamp
//		uint64_t timestamp = 0;
//		for (int i = 0; i < 6; i++) {
//			timestamp = (timestamp << 8) | data[5 + i];
//		}
//		order.timestamp_ns = timestamp;
//
//		//referece number
//		uint64_t order_ref = 0;
//		for (int i = 0; i < 8; i++) {
//			order_ref = (order_ref << 8) | data[11 + i];
//		}
//		order.order_ref = order_ref;
//
//
//		//buy or sell
//		order.side = data[19];
//		uint32_t shares = 0;
//		for (int j = 0; j < 4; j++) {
//			shares = (shares << 8) | data[20 + j];
//		}
//		order.shares = shares;
//
//		//stock symbol 
//		char stock[8];
//		for (int k = 0; k < 8; k++) {
//			order.stock[k] = data[24 + k];
//		}
//		
//
//		//price of unit
//		uint32_t price = 0;
//		for (int i = 0; i < 4; i++) {
//			price = (price << 8) | data[32 + i];
//		}
//		order.price = price / 10000;
//		return order;
//	}
//static Order parse_delete_order(const uint8_t* data) {
//	Order order;
//
//	// Message Type 'D' 
//	order.msg_type = data[0];
//
//
//	uint64_t timestamp = 0;
//	for (int i = 0; i < 6; i++) {
//		timestamp = (timestamp << 8) | data[5 + i];
//	}
//	order.timestamp_ns = timestamp;
//
//
//	uint64_t order_ref = 0;
//	for (int i = 0; i < 8; i++) {
//		order_ref = (order_ref << 8) | data[11 + i];
//	}
//	order.order_ref = order_ref;
//
//	return order;
//}
//
//
//};


#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#define LIKELY(x) (x)
#else
#define FORCE_INLINE __attribute__((always_inline)) inline
#define LIKELY(x) __builtin_expect(!!(x), 1)
#endif



FORCE_INLINE uint16_t read_be16(const uint8_t* p) {
    uint16_t v;
    memcpy(&v, p, 2);
#if defined(_MSC_VER)
    return _byteswap_ushort(v);
#else
    return __builtin_bswap16(v);
#endif
}

FORCE_INLINE uint32_t read_be32(const uint8_t* p) {
    uint32_t v;
    memcpy(&v, p, 4);
#if defined(_MSC_VER)
    return _byteswap_ulong(v);
#else
    return __builtin_bswap32(v);
#endif
}

FORCE_INLINE uint64_t read_be64(const uint8_t* p) {
    uint64_t v;
    memcpy(&v, p, 8);
#if defined(_MSC_VER)
    return _byteswap_uint64(v);
#else
    return __builtin_bswap64(v);
#endif
}

// 6-byte timestamp manual)
FORCE_INLINE uint64_t read_be48(const uint8_t* p) {
    return (uint64_t(p[0]) << 40) |
        (uint64_t(p[1]) << 32) |
        (uint64_t(p[2]) << 24) |
        (uint64_t(p[3]) << 16) |
        (uint64_t(p[4]) << 8) |
        uint64_t(p[5]);
}


FORCE_INLINE void parse_add_order(const uint8_t* data, Order& order) {
    order.msg_type = 'A';
    order.timestamp_ns = read_be48(&data[5]);
    order.order_ref = read_be64(&data[11]);
    order.side = data[19];
    order.shares = read_be32(&data[20]);

    memcpy(order.stock, &data[24], 8);

    order.price = read_be32(&data[32]) * 0.0001;
}

FORCE_INLINE void parse_delete_order(const uint8_t* data, Order& order) {
    order.msg_type = 'D';
    order.timestamp_ns = read_be48(&data[5]);
    order.order_ref = read_be64(&data[11]);

    order.side = 0;
    order.shares = 0;
    order.price = 0.0;
}



using ParserFn = void(*)(const uint8_t*, Order&);

class ITCHParser {
private:
    static ParserFn table[256];

public:
    static void init() {
        for (int i = 0; i < 256; ++i)
            table[i] = nullptr;

        table['A'] = parse_add_order;
        table['D'] = parse_delete_order;
    }

    FORCE_INLINE static void parse(const uint8_t* data, Order& order) {
        ParserFn fn = table[(uint8_t)data[0]];

        if (LIKELY(fn)) {
            fn(data, order);
        }
    }
};

ParserFn ITCHParser::table[256];

