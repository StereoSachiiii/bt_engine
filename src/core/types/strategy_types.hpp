#pragma once
#include <cstdint>
#include <cmath>

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

struct PnLSummary {
    double realized = 0.0;
    double unrealized = 0.0;
    double total = 0.0;
};

struct VirtualOrder {
    uint64_t price;
    uint32_t qty;
    uint32_t volume_ahead; 
    bool active = false;
};

struct StockOrders {
    VirtualOrder bid;
    VirtualOrder ask;
};
