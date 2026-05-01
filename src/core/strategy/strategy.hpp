#pragma once
#include "../order.hpp"
#include "../order_book_manager.hpp"
#include "position_manager.hpp"
#include "pnl_tracker.hpp"

class Strategy {
protected:
    PositionManager pos_mgr_;
    
public:

    void on_order_book_update(uint16_t locate, const OrderBook& book);
    void on_trade(uint16_t locate, double price, uint32_t qty);
    void on_event(const Order& event, const OrderBookManager& manager);

    
    const PositionManager& get_position_manager() const { return pos_mgr_; }
    
    PnLSummary get_pnl(const OrderBookManager& book_mgr) const {
        return PnLTracker::calculate_pnl(pos_mgr_, book_mgr);
    }
};

