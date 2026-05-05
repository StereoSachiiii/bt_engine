#pragma once
#include "../types/order.hpp"
#include "../order_book_manager.hpp"
#include "position_manager.hpp"
#include "pnl_tracker.hpp"

class Strategy {
protected:
    PositionManager pos_mgr_;
    
public:

    
    const PositionManager& get_position_manager() const { return pos_mgr_; }
    
    PnLSummary get_pnl(const OrderBookManager& book_mgr) const {
        return PnLTracker::calculate_pnl(pos_mgr_, book_mgr);
    }
};

