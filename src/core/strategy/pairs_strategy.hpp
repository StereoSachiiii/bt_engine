#pragma once
#include "strategy.hpp"
#include "virtual_matcher.hpp"
#include <iostream>
#include <cmath>
#include <deque>
#include <numeric>

class PairsStrategy : public Strategy {
private:
    uint16_t locate_a_ = 0;
    uint16_t locate_b_ = 0;
    VirtualMatcher matcher_a_;
    VirtualMatcher matcher_b_;

    // Z-Score Window
    std::deque<double> spread_history_;
    const size_t window_size_ = 100;

    
    // Strategy Parameters
    const double entry_z_ = 2.0;
    const double exit_z_ = 0.5;
    const int32_t trade_qty_ = 100;

    struct State {
        double last_mid = 0;
        double bid_quote = 0;
        double ask_quote = 1e9;
    } state_a_, state_b_;

public:
    PairsStrategy(uint16_t a, uint16_t b) : locate_a_(a), locate_b_(b) {}

    void on_event(const Order& event, const OrderBookManager& manager) {
        if (event.stock_locate == locate_a_) matcher_a_.on_message(event, pos_mgr_, manager);
        if (event.stock_locate == locate_b_) matcher_b_.on_message(event, pos_mgr_, manager);
    }

    void on_order_book_update(uint16_t locate, const OrderBook& book) {
        if (locate != locate_a_ && locate != locate_b_) [[likely]] return;

        double mid = book.weighted_mid();
        if (mid == 0) [[unlikely]] return;

        if (locate == locate_a_) state_a_.last_mid = mid;
        else state_b_.last_mid = mid;

       
        if (state_a_.last_mid == 0 || state_b_.last_mid == 0) [[unlikely]] return;

        double spread = state_a_.last_mid - state_b_.last_mid;
        spread_history_.push_back(spread);
        if (spread_history_.size() > window_size_) spread_history_.pop_front();

        if (spread_history_.size() < window_size_) [[unlikely]] return;

        double sum = std::accumulate(spread_history_.begin(), spread_history_.end(), 0.0);
        double mean = sum / spread_history_.size();
        double sq_sum = std::inner_product(spread_history_.begin(), spread_history_.end(), spread_history_.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / spread_history_.size() - mean * mean);
        double z_score = (spread - mean) / (stdev + 0.0001);

        double ofi_z = book.get_ofi_zscore(); 

        int32_t pos_a = (int32_t)pos_mgr_.get_position(locate_a_).net_qty;
        int32_t pos_b = (int32_t)pos_mgr_.get_position(locate_b_).net_qty;

        if (z_score > entry_z_ && pos_a >= 0) [[unlikely]] {
            if (locate == locate_a_ && ofi_z < -2.0) [[unlikely]] { 
                matcher_a_.place_order(locate_a_, 'S', (uint64_t)state_a_.last_mid, trade_qty_, 0);
                matcher_b_.place_order(locate_b_, 'B', (uint64_t)state_b_.last_mid, trade_qty_, 0);
                std::cout << "[Pairs] Entry: SELL A / BUY B (Z: " << z_score << ", OFI_Z: " << ofi_z << ")\n";
            }
        }
        
        else if (z_score < -entry_z_ && pos_a <= 0) [[unlikely]] {
            if (locate == locate_a_ && ofi_z > 2.0) [[unlikely]] {
                matcher_a_.place_order(locate_a_, 'B', (uint64_t)state_a_.last_mid, trade_qty_, 0);
                matcher_b_.place_order(locate_b_, 'S', (uint64_t)state_b_.last_mid, trade_qty_, 0);
                std::cout << "[Pairs] Entry: BUY A / SELL B (Z: " << z_score << ", OFI_Z: " << ofi_z << ")\n";
            }
        }

    }

    void on_trade(uint16_t, double, uint32_t) {}
};
