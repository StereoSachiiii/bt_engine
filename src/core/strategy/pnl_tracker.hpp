#pragma once
#include "position_manager.hpp"
#include "../order_book_manager.hpp"

struct PnLSummary {
    double realized = 0.0;
    double unrealized = 0.0;
    double total = 0.0;
};

class PnLTracker {
public:
    static PnLSummary calculate_pnl(const PositionManager& pos_mgr, const OrderBookManager& book_mgr) {
        PnLSummary summary;
        
        for (const auto& [locate, pos] : pos_mgr.all_positions()) {
            summary.realized += pos.realized_pnl;

            // Calculate  Mid-Price
            const OrderBook* book = const_cast<OrderBookManager&>(book_mgr).get_or_create_book(locate, ' ');
            if (book && pos.net_qty != 0) {
                uint64_t bb = book->best_bid();
                uint64_t ba = book->best_ask();
                
                if (bb > 0 && ba > 0) {
                    double mid_price = (static_cast<double>(bb) + static_cast<double>(ba)) / 2.0;
                    double unrealized = (mid_price - pos.avg_price) * pos.net_qty;
                    summary.unrealized += unrealized;
                }
            }
        }
        
        summary.realized /= 10000.0;
        summary.unrealized /= 10000.0;
        summary.total = summary.realized + summary.unrealized;
        return summary;
    }
};
