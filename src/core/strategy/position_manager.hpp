#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

struct Position {
    int64_t net_qty = 0;
    double avg_price = 0.0;
    double realized_pnl = 0.0;

    void update_on_fill(int64_t fill_qty, double fill_price, bool is_buy) {
        int64_t signed_fill_qty = is_buy ? fill_qty : -fill_qty;
        
        if ((net_qty >= 0 && is_buy) || (net_qty <= 0 && !is_buy)) {
            double total_cost = (net_qty * avg_price) + (signed_fill_qty * fill_price);
            net_qty += signed_fill_qty;
            avg_price = (net_qty != 0) ? total_cost / net_qty : 0.0;
        } else {
            int64_t closing_qty = (std::abs(net_qty) < std::abs(signed_fill_qty)) 
                                  ? std::abs(net_qty) 
                                  : std::abs(signed_fill_qty);
            
            double pnl = (is_buy) 
                         ? (avg_price - fill_price) * closing_qty  // Covering a short
                         : (fill_price - avg_price) * closing_qty; // Selling a long
            
            realized_pnl += pnl;

            int64_t remaining_qty = signed_fill_qty + (is_buy ? closing_qty : -closing_qty);
            net_qty += (is_buy ? closing_qty : -closing_qty);
            
            if (remaining_qty != 0) {
                net_qty = remaining_qty;
                avg_price = fill_price;
            } else if (net_qty == 0) {
                avg_price = 0.0;
            }
        }
    }
};

class PositionManager {
private:
   
    std::unordered_map<uint16_t, Position> positions_;

public:
    void on_fill(uint16_t locate, int64_t qty, double price, bool is_buy) {
        positions_[locate].update_on_fill(qty, price, is_buy);
    }

    const Position& get_position(uint16_t locate) const {
        static const Position empty{};
        auto it = positions_.find(locate);
        return (it != positions_.end()) ? it->second : empty;
    }

    const std::unordered_map<uint16_t, Position>& all_positions() const {
        return positions_;
    }
};
