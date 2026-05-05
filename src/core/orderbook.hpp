#pragma once
#include "config.hpp"
#include "object_pool.hpp"
#include "types/order.hpp"
#include "types/order_book_event.hpp"
#include "types/price_level.hpp"
#include "hierarchical_bitset.hpp"
#include "types/orderbook_memory.hpp"
#include "utils/hash.hpp"
#include <atomic>
#include <cstdint>
#include <array>
#include <memory>
#include <cstring>
#include "utils/compiler.hpp"

class OrderBook {
private:
    static constexpr size_t INDEX_SIZE = config::INDEX_SIZE;
    static constexpr size_t INDEX_MASK = config::INDEX_MASK;
    static constexpr size_t PRICE_WINDOW = config::PRICE_WINDOW;

    OrderBookMemory* memory_ = nullptr;

    uint64_t base_price_ = 0;
    ObjectPool<Order, config::MAX_ORDERS>& order_pool_;
    ObjectPool<PriceLevel, config::MAX_PRICE_LEVELS>& level_pool_;
    char market_category_ = ' ';
    uint32_t order_count_ = 0;

    uint64_t last_bid_price_ = 0;
    uint32_t last_bid_qty_ = 0;
    uint64_t last_ask_price_ = 0;
    uint32_t last_ask_qty_ = 0;
    double ofi_accumulator_ = 0;
    uint64_t total_volume_ = 0;
    uint64_t last_update_ns_ = 0;
    uint64_t out_of_window_drops_ = 0;

public:

    OrderBook(ObjectPool<Order, config::MAX_ORDERS>& order_pool, ObjectPool<PriceLevel, config::MAX_PRICE_LEVELS>& level_pool) 
        : order_pool_(order_pool), level_pool_(level_pool)
    {
        memory_ = static_cast<OrderBookMemory*>(alloc_zeroed_aligned(sizeof(OrderBookMemory)));
        if (memory_) {
            new (&memory_->bid_bits) HierarchicalBitset();
            new (&memory_->ask_bits) HierarchicalBitset();
        }
    }

    ~OrderBook() {
        if (memory_) free_zeroed_aligned(memory_, sizeof(OrderBookMemory));
    }



    void set_market_category(char cat) { market_category_ = cat; }

    FORCE_INLINE void apply_add(uint64_t ref, uint8_t side, uint64_t price, uint32_t qty, uint16_t locate, uint64_t ts_ns) {
        last_update_ns_ = ts_ns;
        if (order_count_ == 0) [[unlikely]] base_price_ = price - (PRICE_WINDOW / 2);

        if (price < base_price_ || price >= base_price_ + PRICE_WINDOW) [[unlikely]] {
            out_of_window_drops_++;
            return;
        }

        Order* o = order_pool_.allocate();
        o->order_ref = ref;
        o->side = side;
        o->price = price;
        o->shares = qty;
        o->stock_locate = locate;
        o->timestamp_ns = ts_ns;
        o->market_category = market_category_;
        o->prev = o->next = nullptr;
        order_count_++;

        bool is_bid = (side == 'B');
        size_t idx = (size_t)(price - base_price_);
        PriceLevel** levels = is_bid ? memory_->price_levels_bid : memory_->price_levels_ask;
        HierarchicalBitset* bits = is_bid ? &memory_->bid_bits : &memory_->ask_bits;

        if (idx >= PRICE_WINDOW) [[unlikely]] return;

        PriceLevel* lvl = levels[idx];
        if (!lvl) [[unlikely]] {
            lvl = level_pool_.allocate();
            lvl->price = price;
            lvl->total_qty = qty;
            lvl->head = lvl->tail = o;
            lvl->order_count = 1;
            levels[idx] = lvl;
            bits->set(idx);
        } else {
            lvl->tail->next = o;
            o->prev = lvl->tail;
            lvl->tail = o;
            lvl->total_qty += qty;
            lvl->order_count++;
        }

        index_insert_impl(ref, o);
        update_ofi();
    }

    FORCE_INLINE void apply_delete(uint64_t ref, uint64_t ts_ns) {
        last_update_ns_ = ts_ns;
        if (!memory_) [[unlikely]] return;

        Order* o = index_find_impl(ref);
        if (!o) [[unlikely]] return;

        bool is_bid = (o->side == 'B');
        size_t idx = (size_t)(o->price - base_price_);
        PriceLevel** levels = is_bid ? memory_->price_levels_bid : memory_->price_levels_ask;
        PriceLevel* lvl = levels[idx];

        if (o->prev) o->prev->next = o->next;
        else         lvl->head = o->next;
        if (o->next) o->next->prev = o->prev;
        else         lvl->tail = o->prev;

        lvl->total_qty -= o->shares;
        if (--lvl->order_count == 0) {
            (is_bid ? &memory_->bid_bits : &memory_->ask_bits)->reset(idx);
            levels[idx] = nullptr;
            level_pool_.deallocate(lvl);
        }

        index_erase_impl(ref);
        order_pool_.deallocate(o);
        if (order_count_ > 0) order_count_--;
        update_ofi();
    }

    FORCE_INLINE void apply_execute(uint64_t ref, uint32_t qty, uint64_t ts_ns) {
        last_update_ns_ = ts_ns;
        total_volume_ += qty;
        apply_cancel(ref, qty, ts_ns);
    }

    FORCE_INLINE void apply_cancel(uint64_t ref, uint32_t qty, uint64_t ts_ns) {
        last_update_ns_ = ts_ns;
        if (!memory_) [[unlikely]] return;

        Order* o = index_find_impl(ref);
        if (!o) [[unlikely]] return;

        if (qty >= o->shares) {
            apply_delete(ref, ts_ns);
        } else {
            o->shares -= qty;
            size_t idx = (size_t)(o->price - base_price_);
            (o->side == 'B' ? memory_->price_levels_bid : memory_->price_levels_ask)[idx]->total_qty -= qty;
        }
        update_ofi();
    }

    void apply_execute(uint64_t ref, uint32_t qty) { apply_cancel(ref, qty); }


    FORCE_INLINE void apply_replace(uint64_t old_ref, uint64_t new_ref, uint32_t new_qty, uint64_t new_price, uint64_t ts_ns) {
        last_update_ns_ = ts_ns;
        Order* old_order = index_find_impl(old_ref);
        if (!old_order) [[unlikely]] return;
        uint8_t side = old_order->side;
        uint16_t locate = old_order->stock_locate;
        apply_delete(old_ref);
        apply_add(new_ref, side, new_price, new_qty, locate, ts_ns);
        update_ofi();
    }


    FORCE_INLINE Order* get_order(uint64_t ref) { return index_find_impl(ref); }


    FORCE_INLINE uint64_t best_bid() const {
        if (!memory_) [[unlikely]] return 0;
        int idx = memory_->bid_bits.find_last();
        return (idx != -1) ? memory_->price_levels_bid[idx]->price : 0;
    }

    FORCE_INLINE uint64_t best_ask() const {
        if (!memory_) [[unlikely]] return 0;
        int idx = memory_->ask_bits.find_first();
        return (idx != -1) ? memory_->price_levels_ask[idx]->price : 0;
    }

    FORCE_INLINE uint32_t bid_qty() const {
        if (!memory_) [[unlikely]] return 0;
        int idx = memory_->bid_bits.find_last();
        return (idx != -1) ? (uint32_t)memory_->price_levels_bid[idx]->total_qty : 0;
    }

    FORCE_INLINE uint32_t ask_qty() const {
        if (!memory_) [[unlikely]] return 0;
        int idx = memory_->ask_bits.find_first();
        return (idx != -1) ? (uint32_t)memory_->price_levels_ask[idx]->total_qty : 0;
    }

    
    FORCE_INLINE void update_ofi() {
        uint64_t current_bid = best_bid();
        uint32_t current_bid_qty = bid_qty();
        uint64_t current_ask = best_ask();
        uint32_t current_ask_qty = ask_qty();

        double delta_bid = 0;
        if (current_bid > last_bid_price_) {
            delta_bid = (double)current_bid_qty;
        } else if (current_bid == last_bid_price_) {
            delta_bid = (double)current_bid_qty - last_bid_qty_;
        } else {
            delta_bid = -(double)last_bid_qty_; // Level dropped
        }

        double delta_ask = 0;
        if (current_ask < last_ask_price_) {
            delta_ask = (double)current_ask_qty;
        } else if (current_ask == last_ask_price_) {
            delta_ask = (double)current_ask_qty - last_ask_qty_;
        } else {
            delta_ask = -(double)last_ask_qty_; // Level rose
        }

        ofi_accumulator_ += (delta_bid - delta_ask);

        last_bid_price_ = current_bid;
        last_bid_qty_ = current_bid_qty;
        last_ask_price_ = current_ask;
        last_ask_qty_ = current_ask_qty;
    }

    FORCE_INLINE double get_ofi() const { return ofi_accumulator_; }
    FORCE_INLINE double get_normalized_ofi() const {
        if (total_volume_ == 0) return 0.0;
        return ofi_accumulator_ / (double)total_volume_;
    }

    FORCE_INLINE double get_imbalance() const {
        uint32_t b = bid_qty();
        uint32_t a = ask_qty();
        if (b == 0 && a == 0) return 0.0;
        return (double(b) - double(a)) / (double(b) + double(a));
    }

    double get_book_imbalance(int max_levels) const {
        double b_total = 0, a_total = 0;
        int found = 0;
        int idx = memory_->bid_bits.find_last();
        while (idx != -1 && found < max_levels) {
            b_total += memory_->price_levels_bid[idx]->total_qty;
            idx = -1; // TODO: implement find_next_last for deeper book
            found++;
        }
        found = 0;
        idx = memory_->ask_bits.find_first();
        while (idx != -1 && found < max_levels) {
            a_total += memory_->price_levels_ask[idx]->total_qty;
            idx = -1; // TODO: implement find_next_first for deeper book
            found++;
        }
        if (b_total + a_total == 0) return 0.0;
        return (b_total - a_total) / (b_total + a_total);
    }

    uint64_t get_total_volume() const { return total_volume_; }
    uint64_t get_last_update_ns() const { return last_update_ns_; }
    uint64_t get_drops() const { return out_of_window_drops_; }

    FORCE_INLINE double weighted_mid() const {
        uint64_t bb = best_bid();
        uint64_t ba = best_ask();
        uint32_t bq = bid_qty();
        uint32_t aq = ask_qty();
        if (bq == 0 || aq == 0) return 0.0;
        return (double(bb) * aq + double(ba) * bq) / (bq + aq);
    }

    //delete everything copy and move
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(const OrderBook&&) = delete;
    OrderBook& operator=(const OrderBook&&) = delete;

private:
    FORCE_INLINE void index_insert_impl(uint64_t ref, Order* o) {
        size_t h = hash_util::murmur64(ref) & INDEX_MASK;
        while (memory_->order_index[h].ref != 0) h = (h + 1) & INDEX_MASK;
        memory_->order_index[h] = { ref, o };
    }

    FORCE_INLINE Order* index_find_impl(uint64_t ref) {
        size_t h = hash_util::murmur64(ref) & INDEX_MASK;
        while (memory_->order_index[h].ref != 0) {
            if (memory_->order_index[h].ref == ref) return memory_->order_index[h].order;
            h = (h + 1) & INDEX_MASK;
        }
        return nullptr;
    }

    FORCE_INLINE void index_erase_impl(uint64_t ref) {
        size_t i = hash_util::murmur64(ref) & INDEX_MASK;
        while (memory_->order_index[i].ref != 0) {
            if (memory_->order_index[i].ref == ref) {
                memory_->order_index[i].ref = 0;
                memory_->order_index[i].order = nullptr;
                
                size_t j = i;
                while (true) {
                    j = (j + 1) & INDEX_MASK;
                    if (memory_->order_index[j].ref == 0) break;
                    
                    size_t k = hash_util::murmur64(memory_->order_index[j].ref) & INDEX_MASK;
             
                    if (i <= j) {
                        if (!(i < k && k <= j)) {
                            memory_->order_index[i] = memory_->order_index[j];
                            memory_->order_index[j].ref = 0;
                            memory_->order_index[j].order = nullptr;
                            i = j;
                        }
                    } else {
                        if (!(i < k || k <= j)) {
                            memory_->order_index[i] = memory_->order_index[j];
                            memory_->order_index[j].ref = 0;
                            memory_->order_index[j].order = nullptr;
                            i = j;
                        }
                    }
                }
                return;
            }
            i = (i + 1) & INDEX_MASK;
        }
    }
};
